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
      //potential_producer_window.at(idx).insert(std::pair<IntPtr, uint64_t>(0,0));
      //correlation_table.at(idx).resize(correlation_table_size, {0,0,NULL});
      //prefetch_request_queue.at(idx).insert(std::pair<IntPtr, IntPtr>(0,0));

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
    //bool ct_found=false;
	IntPtr CN = dynins->eip;
	IntPtr PR = 0;
    bool already_in_ppw = false;

    //only deal with memory read, whose target reg is not empty
    if ( dynins->num_target_reg != 0)
    {

        //String inst_template = dynins->instruction->getDisassembly();

        /* The outest vector is entry number */
        std::map<IntPtr, uint64_t>   &ppw = potential_producer_window.at(flows_per_core ? _core_id - core_id : 0);
        std::map<IntPtr, uint64_t> temp_ppw;
        std::vector <correlation_entry>      &ct = correlation_table.at(flows_per_core ? _core_id - core_id : 0);
        correlation_entry temp_ct(0,0,"");
        //std::unordered_map<IntPtr, IntPtr>   &prq = prefetch_request_queue.at(flows_per_core ? _core_id - core_id : 0);
        //Cache*                               &pb = prefetch_buffer.at(flows_per_core ? _core_id - core_id : 0);

        cout<<endl;
        cout<<endl;
        cout<<"In function: "<<__func__<<" current_address is  "<<hex<<current_address;
        cout<<" After add offset: "<<" current_address is  "<<hex<<current_address+offset<<endl;
        cout<<" diss is: "<<itostr( dynins->instruction->getDisassembly()).c_str()<<" eip is: "<<hex<<dynins->eip<<" dynins pointer is: "<<dynins<<endl;

        std::string inst_temp = dynins->instruction->getDisassembly().c_str();

        // compute the offset of load instruction to get the real base reg value,
        // which will be used to compare with  other inst's target reg in PPW
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
        //step 1: find producer in PPW, the base address is the whole address(current_address+offset) - inst_offset
        cout<<"STEP 1:  find producer in PPW"<<endl;
        for (std::map<IntPtr, uint64_t>::iterator it = ppw.begin(); it!=ppw.end(); it++)
        {
            if(it->second == current_address+offset-inst_offset)
            {
                //if ppw has more than one hit, multihit
                //assert(ppw_found==false);
                ppw_found=true;
                //record and used to get the current inst's producer, which is the nearest one smaller then current one
                //cout<<"Inserting temp_ppw PC is: "<<it->first<<" TargetValue is:"<<it->second<<endl;
                temp_ppw.insert(std::make_pair(it->first, it->second));
            }
        }

        //step 2 put PR/CN/TMPL into CT, hit in PPW
        cout<<"STEP 2:  if hit in PPW, update CT"<<endl;
        if( ppw_found == true )
        {
            //insert the current inst, the map will order them automatically
            //for (std::map<IntPtr, uint64_t>::iterator it = temp_ppw.begin(); it!=temp_ppw.end(); it++)
            //{
            //    cout<<"In temp_ppw PC is: "<<it->first<<" TargetValue is:"<<it->second<<endl;
            //}



            cout<<"temp_ppw size is "<<temp_ppw.size()<<endl;
            for (std::map<IntPtr, uint64_t>::iterator it = temp_ppw.begin(); it!=temp_ppw.end(); it++)
            {
                cout<<"In temp_ppw PC is: "<<it->first<<" TargetValue is:"<<it->second<<endl;
            }

            //find the nearest one as the PR, and the PR must smaller than current inst
            // the smaller one has higher priority
            for (std::map<IntPtr, uint64_t>::reverse_iterator rit = temp_ppw.rbegin(); rit!=temp_ppw.rend(); rit++)
            {
                cout<<"In temp_ppw PC is: "<<hex<<rit->first<<" rit->second is "<<rit->second<<endl;
                if (rit->first <= dynins->eip)
                {
                    PR = rit->first;
                    cout<<"For inst: "<<dynins->eip<<" Producer is: "<<hex<<PR<<" TargetValue is:"<<rit->second<<endl;
                    break;
                }
            }
            if (!PR)
            {
                for (std::map<IntPtr, uint64_t>::iterator it = temp_ppw.begin(); it!=temp_ppw.end(); it++)
                {
                    cout<<"In temp_ppw PC is: "<<hex<<it->first<<" it->second is "<<it->second<<endl;
                    if (it->first > dynins->eip)
                    {
                        PR = it->first;
                        cout<<"For inst: "<<dynins->eip<<" Producer is: "<<hex<<PR<<" TargetValue is:"<<it->second<<endl;
                        break;
                    }
                }
            }
            temp_ppw.clear();

            //cout<<"Before insert in ct: ct size is:"<<dec<<ct.size()<<endl;
            //for (std::vector<correlation_entry>::iterator iter=ct.begin(); iter!=ct.end(); iter++)
            //{
            //    cout<<"In CT Producer is: "<<hex<<iter->GetProducerPC()<<" ConsumerPC is "<<iter->GetConsumerPC()<<" template is: "<<iter->GetDisass()<<endl;
            //}

            temp_ct.SetCT(PR, CN, inst_temp);
            if (ct.size()>=correlation_table_size)
            {
                ct.pop_back();
            }
            ct.push_back(temp_ct);


            cout<<"After insert in ct:"<<endl;
            //print ct
            for (std::vector<correlation_entry>::iterator iter=ct.begin(); iter!=ct.end(); iter++)
            {
                cout<<"In CT Producer is: "<<hex<<iter->GetProducerPC()<<" ConsumerPC is "<<iter->GetConsumerPC()<<" template is: "<<iter->GetDisass()<<endl;
            }
        }
        //step 3, insert to ppw
        cout<<"STEP 3:  update PPW"<<endl;
        for (std::map<IntPtr, uint64_t>::iterator it_ppw = ppw.begin(); it_ppw!=ppw.end(); it_ppw++)
        {
            if (( it_ppw->first == dynins->eip) && (it_ppw->second == dynins->target_reg[dynins->num_target_reg-1]))
            {
                already_in_ppw = true;
                cout<<"In ppw already have one "<<hex<<dynins->eip<<" target reg is "<<hex<<dynins->target_reg[dynins->num_target_reg-1] <<endl;
            }
        }
        if (!already_in_ppw)
        {
            if(ppw.size() >= potential_producer_window_size)
            {
                std::map<IntPtr, uint64_t>::iterator it_delete = ppw.end();
                it_delete--;
                //TODO: ppw replacement algorithm should be updated
                cout<<"PPW is full, erasing: "<<it_delete->first<<endl;;
                ppw.erase(it_delete);
            }
            //if base address is sp, do NOT enter PPW
            //cout<<dynins->instruction->getDisassembly().find("push") <<endl;
            //cout<<dynins->instruction->getDisassembly().find("pop") <<endl;
            if( dynins->instruction->getDisassembly().find("push") == string::npos &&
                    dynins->instruction->getDisassembly().find("pop") == string::npos)
            {
                //the most right reg is the target reg loaded from memory
                cout<<"Inserting to ppw: "<<hex<<dynins->eip<<" target reg is "<<hex<<dynins->target_reg[dynins->num_target_reg-1] <<endl;
                pair<std::map<IntPtr, uint64_t>::iterator, bool> ppw_return=ppw.insert(std::make_pair(dynins->eip, dynins->target_reg[dynins->num_target_reg-1]));
                //if current ip already in the PPW, update it.
                if(ppw_return.second == false)
                {
                    ppw.erase(dynins->eip);
                    ppw.insert(std::make_pair(dynins->eip, dynins->target_reg[dynins->num_target_reg-1]));
                }


#if 0
                cout<<"After insert in ppw:"<<endl;
                //print ppw
                for (std::map<IntPtr, uint64_t>::iterator it = ppw.begin(); it!=ppw.end(); it++)
                {
                    cout<<"In PPW PC is: "<<hex<<it->first<<" TargetValue is "<<it->second<<endl;
                }
#endif
            }
        }

        //step 4, lookup CT to get the next prefetch address
        //PC as producer to get potential consumer
        cout<<"STEP 4:  get the prefetch address from CT"<<endl;
        std::vector<IntPtr> addresses;
        cout<<"Current CT is: "<<endl;
        //print ct
        //for (std::vector<correlation_entry>::iterator iter=ct.begin(); iter!=ct.end(); iter++)
        //{
        //    cout<<"In CT Producer is: "<<hex<<iter->GetProducerPC()<<" ConsumerPC is "<<iter->GetConsumerPC()<<" template is: "<<iter->GetDisass()<<endl;
        //}

        //for (uint32_t k=0; k<correlation_table_size; k++)
        for (std::vector<correlation_entry>::iterator iter=ct.begin(); iter!=ct.end(); iter++)
        {
            //To get the offset for every comsumer inst, used to compute the prefetch address
            if (iter->GetProducerPC() == dynins->eip)
            {
                std::string inst_temp1 = iter->GetDisass();
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

                    }
                }

                cout<<" inst_offset_cn 0x"<<hex<<inst_offset_cn<<endl;
                //get consumer PC, may be multiple
                IntPtr prefetch_address = dynins->target_reg[dynins->num_target_reg-1] + inst_offset_cn;
                // But stay within the page if requested
                if ((!stop_at_page || ((prefetch_address & PAGE_MASK) == (current_address & PAGE_MASK))) && addresses.size() < num_prefetches)
                {
                    if (prefetch_address > 0xfffff)
                    {
                        addresses.push_back(prefetch_address);
                        cout<<"Prefetch address for "<<hex<<dynins->eip<<" is "<<hex<<prefetch_address<<endl;
                    }
                }
            }
        }


        return addresses;
    }
    std::vector<IntPtr> empty_addresses;
    return empty_addresses;
}
