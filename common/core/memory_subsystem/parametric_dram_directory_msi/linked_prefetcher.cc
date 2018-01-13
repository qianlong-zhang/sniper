#include "linked_prefetcher.h"
#include "simulator.h"
#include "config.hpp"
#include "instruction.h"
#include <string>

#include <cstdlib>

const IntPtr PAGE_SIZE = 4096;
const IntPtr PAGE_MASK = ~(PAGE_SIZE-1);

LinkedPrefetcher::LinkedPrefetcher(String configName, core_id_t _core_id, UInt32 _shared_cores)
   : core_id(_core_id)
   , shared_cores(_shared_cores)
   , n_flows(Sim()->getCfg()->getIntArray("perf_model/" + configName + "/prefetcher/linked/flows", core_id))
   , flows_per_core(Sim()->getCfg()->getBoolArray("perf_model/" + configName + "/prefetcher/linked/flows_per_core", core_id))
   , num_prefetches(Sim()->getCfg()->getIntArray("perf_model/" + configName + "/prefetcher/linked/num_prefetches", core_id))
   , stop_at_page(Sim()->getCfg()->getBoolArray("perf_model/" + configName + "/prefetcher/linked/stop_at_page_boundary", core_id))
   , n_flow_next(0)
   , m_prev_address(flows_per_core ? shared_cores : 1)
   , potential_producer_window(flows_per_core ? shared_cores : 1)
   , correlation_table(flows_per_core ? shared_cores : 1)
   , prefetch_request_queue(flows_per_core ? shared_cores : 1)
   , prefetch_buffer(flows_per_core ? shared_cores : 1)
   , potential_producer_window_size(Sim()->getCfg()->getIntArray("perf_model/" + configName + "/prefetcher/linked/potential_producer_window_size", core_id))  /*those param are only used to limit the size of the queue.*/
   , correlation_table_size(Sim()->getCfg()->getIntArray("perf_model/" + configName + "/prefetcher/linked/correlation_table_size", core_id))
   , prefetch_request_queue_size(Sim()->getCfg()->getIntArray("perf_model/" + configName + "/prefetcher/linked/prefetch_request_queue_size", core_id))
   , prefetch_buffer_size(Sim()->getCfg()->getIntArray("perf_model/" + configName + "/prefetcher/linked/prefetch_buffer_size", core_id))
{
   for(UInt32 idx = 0; idx < (flows_per_core ? shared_cores : 1); ++idx)
   	{
      m_prev_address.at(idx).resize(n_flows);
      potential_producer_window.at(idx).insert(std::pair<IntPtr, uint64_t>(0,0));
      correlation_table.at(idx).resize(correlation_table_size, {0,0,NULL});
      prefetch_request_queue.at(idx).insert(std::pair<IntPtr, IntPtr>(0,0));

      //for (UInt32 i = 0; i<potential_producer_window_size; i++)
      //{
      //    potential_producer_window.at(idx).insert(std::pair<IntPtr,uint64_t>(0,0));
      //}
      //for (UInt32 j = 0; j<correlation_table_size; j++)
      //{
      //    correlation_entry temp_ct(0,0,NULL);
      //    correlation_table.at(idx).push_back(temp_ct);
      //}
	  //correlation_table.at(idx).resize(correlation_table_size);
	  //prefetch_request_queue.at(idx).resize(prefetch_request_queue_size);
	  //prefetch_buffer.at(idx).resize(prefetch_buffer_size);

#if 0
    /* For linked prefetcher, create PPW/CT/PRQ/PB */
      prefetch_buffer.at(_core_id) = new Cache(configName,
                    configName,
                    _core_id,
                    1,/* set */
                    prefetch_buffer_size,/* associativity */
                    64,
                    "lru",
                    CacheBase::PR_L1_CACHE,
                    CacheBase::parseAddressHash("mask"),
                    NULL);

#endif
   	}
}

std::vector<IntPtr>
LinkedPrefetcher::getNextAddress(IntPtr current_address, UInt32 offset, core_id_t _core_id, DynamicInstruction *dynins)
{
    int32_t ppw_found=false;
    bool ct_found=false;
	IntPtr CN = dynins->eip;
	IntPtr PR = 0;

    //only deal with memory read, whose target reg is not empty
    if ( dynins->num_target_reg != 0)
    {

        String inst_template = dynins->instruction->getDisassembly();

        /* The outest vector is entry number */
        std::map<IntPtr, uint64_t>   &ppw = potential_producer_window.at(flows_per_core ? _core_id - core_id : 0);
        std::vector <correlation_entry>      &ct = correlation_table.at(flows_per_core ? _core_id - core_id : 0);
        //std::unordered_map<IntPtr, IntPtr>   &prq = prefetch_request_queue.at(flows_per_core ? _core_id - core_id : 0);
        //Cache*                               &pb = prefetch_buffer.at(flows_per_core ? _core_id - core_id : 0);

        cout<<endl;
        cout<<endl;
        cout<<"In function: "<<__func__<<" current_address is  "<<hex<<current_address;
        cout<<" After add offset: "<<" current_address is  "<<hex<<current_address+offset<<endl;
        cout<<" diss is: "<<itostr( dynins->instruction->getDisassembly()).c_str()<<" eip is: "<<hex<<dynins->eip<<endl;




        std::string inst_temp = dynins->instruction->getDisassembly().c_str();

        // compute the offset of load instruction to compute the next prefetch address
        string::size_type index1 = inst_temp.find_first_of("]", 0);
        string::size_type index2 = inst_temp.find_first_of("+");
        string::size_type index3 = inst_temp.find_first_of("-");
        string::size_type temp_index;

        //cout<<"index1 is: "<<dec<<index1<<endl;
        //cout<<"index2 is: "<<dec<<index2<<endl;
        //cout<<"index3 is: "<<dec<<index3<<endl;
        int32_t inst_offset=0;
        if (index1 !=string::npos )
        {
            if(!((index2==string::npos) && (index3==string::npos)))
            {
                if (index2 != string::npos)
                    temp_index=index2;
                else
                    temp_index=index3;

                //cout<<temp_index<<endl;
                std::string sub_str = inst_temp.substr(temp_index+1, index1 - temp_index-1);
                //cout<<sub_str<<endl;
                stringstream offset(sub_str);
                offset>>hex>>inst_offset;
                //cout<<dec<<inst_offset<<endl;

                if(index3!=string::npos)
                    inst_offset=-inst_offset;

                //cout<<dec<<inst_offset<<endl;
            }
        }

        cout<<"The real base reg is: "<<hex<<current_address+offset-inst_offset<<endl;
        for (std::map<IntPtr, IntPtr>::iterator it = ppw.begin(); it!=ppw.end(); it++)
        {
            //step 1: find producer in PPW, the base address is the whole address(current_address+offset) - inst_offset
            if(it->second == current_address+offset-inst_offset)
            {
                //if ppw has more than one hit, multihit
                //assert(ppw_found==false);
                PR = it->first;
                ppw_found=true;
                cout<<"For inst: "<<dynins->eip<<" Producer is: "<<hex<<PR<<" TargetValue and base address is:"<<it->second<<endl;
                break;
            }
        }

        //hit in PPW
        if( ppw_found == true )
        {
            //step 2 put PR/CN/TMPL into CT
            for (uint32_t j=0; j<correlation_table_size; j++)
            {
                //found empty one
                if(!ct.at(j).GetProducerPC())
                {
                    ct.at(j).SetCT(PR, CN, dynins);
                    ct_found = true;
                    break;
                }
            }
            if (!ct_found)
            {
                //delete one CT entry-the last one, and insert
                assert(ct.at(correlation_table_size-1).GetProducerPC() != 0);
                ct.at(correlation_table_size-1).SetCT(PR, CN, dynins);
            }


            cout<<"After insert in ct:"<<endl;
            //print ct
            for (uint32_t j=0; j<correlation_table_size; j++)
            {
                if(ct.at(j).GetProducerPC()!=0)
                    cout<<"In CT Producer is: "<<ct.at(j).GetProducerPC()<<" ConsumerPC is "<<ct.at(j).GetConsumerPC()<<" template is: "<<itostr( dynins->instruction->getDisassembly()).c_str()<<endl;
            }
        }
        //step 3, insert to ppw
        if(ppw.size() >= potential_producer_window_size)
        {
            cout<<"PPW is full, erasing: "<<ppw.begin()->first<<endl;;
            ppw.erase(ppw.begin());
        }
        //if base address is sp, do NOT enter PPW
        //cout<<dynins->instruction->getDisassembly().find("push") <<endl;
        //cout<<dynins->instruction->getDisassembly().find("pop") <<endl;
        if( dynins->instruction->getDisassembly().find("push") == string::npos &&
             dynins->instruction->getDisassembly().find("pop") == string::npos)
        {
            cout<<"In line"<<dec<<__LINE__<<" Inserting inst in ppw "<<endl;
            //the most right reg is the target reg loaded from memory
            cout<<"Inserting "<<hex<<dynins->eip<<" target reg is "<<hex<<dynins->target_reg[dynins->num_target_reg-1] <<endl;
            ppw.insert(std::make_pair(dynins->eip, dynins->target_reg[dynins->num_target_reg-1]));
        }
        cout<<"After insert in ppw:"<<endl;
        //print ppw
        for (std::map<IntPtr, uint64_t>::iterator it = ppw.begin(); it!=ppw.end(); it++)
        {
            //if (it->first != 0)
                cout<<"In PPW PC is: "<<hex<<it->first<<" TargetValue is "<<it->second<<endl;
        }

        //step 4, lookup CT to get the next prefetch address
        //PC as producer to get potential consumer
        std::vector<IntPtr> addresses;

        for (uint32_t k=0; k<correlation_table_size; k++)
        {
            //To get the offset for every comsumer inst, used to compute the prefetch address
            if (ct.at(k).GetProducerPC() == dynins->eip)
            {
                std::string inst_temp1 = ct.at(k).GetDynins()->instruction->getDisassembly().c_str();
                // compute the offset of load instruction to compute the next prefetch address
                string::size_type index4 = inst_temp1.find_first_of("]", 0);
                string::size_type index5 = inst_temp1.find_first_of("+");
                string::size_type index6 = inst_temp1.find_first_of("-");
                string::size_type temp_index1;

                int32_t inst_offset_cn=0;
                if (index4 !=string::npos )
                {
                    if(!((index5==string::npos) && (index6==string::npos)))
                    {
                        if (index5 != string::npos)
                            temp_index1=index5;
                        else
                            temp_index1=index6;

                        std::string sub_str = inst_temp1.substr(temp_index1+1, index4 - temp_index1-1);
                        //cout<<sub_str<<endl;
                        stringstream offset(sub_str);
                        offset>>hex>>inst_offset_cn;
                        //cout<<dec<<inst_offset<<endl;

                        if(index6!=string::npos)
                            inst_offset_cn=-inst_offset_cn;

                        cout<<" inst_offset_cn "<<dec<<inst_offset_cn<<endl;
                    }
                }

                //get consumer PC, may be multiple
                IntPtr prefetch_address = dynins->target_reg[dynins->num_target_reg-1] + inst_offset_cn;
                cout<<"Prefetch address for "<<dynins->eip<<" is "<<hex<<prefetch_address<<endl;
                // But stay within the page if requested
                if ((!stop_at_page || ((prefetch_address & PAGE_MASK) == (current_address & PAGE_MASK))) && addresses.size() < num_prefetches)
                    addresses.push_back(prefetch_address);
            }
        }


        return addresses;
    }
    std::vector<IntPtr> empty_addresses;
    return empty_addresses;
}
