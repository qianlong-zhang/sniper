#include "linked_prefetcher.h"
#include "simulator.h"
#include "config.hpp"
#include "instruction.h"
#include <string>

#include <cstdlib>

const IntPtr PAGE_SIZE = 4096;
const IntPtr PAGE_MASK = ~(PAGE_SIZE-1);
#define DEBUG
//#define INFINITE_CT
//#define INFINITE_PPW

LinkedPrefetcher::LinkedPrefetcher(String configName, core_id_t _core_id, UInt32 _shared_cores)
   : core_id(_core_id)
   , shared_cores(_shared_cores)
   , n_flows(Sim()->getCfg()->getIntArray("perf_model/" + configName + "/prefetcher/linked/flows", core_id))
   , flows_per_core(Sim()->getCfg()->getBoolArray("perf_model/" + configName + "/prefetcher/linked/flows_per_core", core_id))
   , num_prefetches(Sim()->getCfg()->getIntArray("perf_model/" + configName + "/prefetcher/linked/num_prefetches", core_id))
   , stop_at_page(Sim()->getCfg()->getBoolArray("perf_model/" + configName + "/prefetcher/linked/stop_at_page_boundary", core_id))
   , cache_block_size(Sim()->getCfg()->getIntArray("perf_model/" + configName + "/cache_block_size", core_id))
   , only_count_lds(Sim()->getCfg()->getBoolArray("perf_model/" + configName + "/prefetcher/linked/only_count_lds", core_id))
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
LinkedPrefetcher::getNextAddress(IntPtr current_address, UInt32 offset, core_id_t _core_id, DynamicInstruction *dynins, UInt64 *pointer_loads)
{
    int32_t ppw_found=false;
    //bool ct_found=false;
	IntPtr PR = 0;
    bool already_in_ppw = false;
    bool already_in_ct = false;

    //if dynins = 0, that means, this memory access is send by doPrefetch(), so we not prefetch for them again
    if (dynins!=0 && dynins->num_target_reg != 0)
    {
        IntPtr CN = dynins->eip;
#ifdef DEBUG
        //cout<<"In func: "<<__func__<<" line "<<dec<<__LINE__<<" dynins = "<<hex<<dynins<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;
#endif
        //only deal with memory read, whose target reg is not empty

        //String inst_template = dynins->instruction->getDisassembly();

        /* The outest vector is entry number */
        std::map<IntPtr, uint64_t>   &ppw = potential_producer_window.at(flows_per_core ? _core_id - core_id : 0);
        std::map<IntPtr, uint64_t> temp_ppw;
        std::vector <correlation_entry>      &ct = correlation_table.at(flows_per_core ? _core_id - core_id : 0);
        correlation_entry temp_ct(0,0,"");
        //std::unordered_map<IntPtr, IntPtr>   &prq = prefetch_request_queue.at(flows_per_core ? _core_id - core_id : 0);
        //Cache*                               &pb = prefetch_buffer.at(flows_per_core ? _core_id - core_id : 0);

#ifdef DEBUG
        cout<<endl;
        cout<<endl;
        cout<<"In function: "<<__func__<<" current_address is  "<<hex<<current_address;
        cout<<" After add offset: "<<" current_address is  "<<hex<<current_address+offset<<endl;
        cout<<" diss is: "<<itostr( dynins->instruction->getDisassembly()).c_str()<<" eip is: "<<hex<<dynins->eip<<endl;
#endif

        std::string inst_temp = dynins->instruction->getDisassembly().c_str();

        // compute the offset of load instruction to get the real base reg value,
        // which will be used to compare with  other inst's target reg in PPW
        string::size_type index1 = inst_temp.find_first_of("]", 0);
        string::size_type index2 = inst_temp.find_first_of("+");
        string::size_type index3 = inst_temp.find_first_of("-");
        string::size_type temp_index;

        int32_t inst_offset=0;
        if (index1 !=string::npos )
        {
            if(!((index2==string::npos) && (index3==string::npos)))
            {
                if (index2 != string::npos)
                    temp_index=index2;
                else
                    temp_index=index3;

                std::string sub_str = inst_temp.substr(temp_index+1, index1 - temp_index-1);
                stringstream offset(sub_str);
                offset>>hex>>inst_offset;

                if(index3!=string::npos)
                    inst_offset=-inst_offset;
#ifdef DEBUG
                //cout<<temp_index<<endl;
                cout<<sub_str<<endl;
                cout<<dec<<inst_offset<<endl;
#endif

            }
        }

#ifdef DEBUG
        cout<<"The real base reg is: "<<hex<<current_address+offset-inst_offset<<endl;
        cout<<"STEP 1:  find producer in PPW";
#endif
        //step 1: find producer in PPW, the base address is the whole address(current_address+offset) - inst_offset
        for (std::map<IntPtr, uint64_t>::iterator it = ppw.begin(); it!=ppw.end(); it++)
        {
            if(it->second == current_address+offset-inst_offset)
            {
                //if ppw has more than one hit, multihit
                //assert(ppw_found==false);
                ppw_found=true;
                //record in temp_ppw to get the current inst's producer, which is the nearest one smaller then current one
#ifdef DEBUG
                cout<<" Found in PPW"<<endl;
                //cout<<"Inserting temp_ppw PC is: "<<it->first<<" TargetValue is:"<<it->second<<endl;
#endif
                temp_ppw.insert(std::make_pair(it->first, it->second));
            }
        }

        //step 2 put PR/CN/TMPL into CT, hit in PPW
#ifdef DEBUG
        cout<<"STEP 2:  if hit in PPW, update CT"<<endl;
#endif
        if( ppw_found == true )
        {
            if (pointer_loads)
                (*pointer_loads)++;
#ifdef DEBUG
            cout<<"temp_ppw size is "<<temp_ppw.size()<<endl;
            for (std::map<IntPtr, uint64_t>::iterator it = temp_ppw.begin(); it!=temp_ppw.end(); it++)
            {
                cout<<"In temp_ppw PC is: "<<it->first<<" TargetValue is:"<<it->second<<endl;
            }
#endif

            // find the nearest one as the PR, and the PR should smaller than current inst
            // the smaller one has higher priority, find the first smaller one from the end
            for (std::map<IntPtr, uint64_t>::reverse_iterator rit = temp_ppw.rbegin(); rit!=temp_ppw.rend(); rit++)
            {
#ifdef DEBUG
                cout<<"In temp_ppw PC is: "<<hex<<rit->first<<" rit->second is "<<rit->second<<endl;
#endif
                if (rit->first <= dynins->eip)
                {
                    PR = rit->first;
#ifdef DEBUG
                    cout<<"For inst: "<<dynins->eip<<" Producer is: "<<hex<<PR<<" TargetValue is:"<<rit->second<<endl;
#endif
                    break;
                }
            }
            //if not found, find a nearest bigger one from the begain
            if (!PR)
            {
                for (std::map<IntPtr, uint64_t>::iterator it = temp_ppw.begin(); it!=temp_ppw.end(); it++)
                {
#ifdef DEBUG
                    cout<<"In temp_ppw PC is: "<<hex<<it->first<<" it->second is "<<it->second<<endl;
#endif
                    if (it->first > dynins->eip)
                    {
                        PR = it->first;
#ifdef DEBUG
                        cout<<"For inst: "<<dynins->eip<<" Producer is: "<<hex<<PR<<" TargetValue is:"<<it->second<<endl;
#endif
                        break;
                    }
                }
            }
            temp_ppw.clear();

            temp_ct.SetCT(PR, CN, inst_temp);
#ifndef INFINITE_CT
           if (ct.size()>=correlation_table_size)
           {
               ct.pop_back();
           }
#endif


            for (std::vector<correlation_entry>::iterator iter1=ct.begin(); iter1!=ct.end(); iter1++)
            {
                //cout<<"Before insert In CT Producer is: "<<hex<<iter1->GetProducerPC()<<" ConsumerPC is "<<iter1->GetConsumerPC()<<" template is: "<<iter1->GetDisass()<<endl;
                if((iter1->GetProducerPC() == PR)&&(iter1->GetConsumerPC() == CN))
                {
                    already_in_ct = true;
#ifdef DEBUG
                    cout<<"In ct already have one PR is: "<<hex<<PR<<" CN is "<<hex<< CN <<endl;
#endif
                }
            }
            if (!already_in_ct &&
                    dynins->instruction->getDisassembly().find("push") == string::npos &&
                    dynins->instruction->getDisassembly().find("pop") == string::npos)
            {
                ct.push_back(temp_ct);
            }
#ifdef DEBUG
            cout<<"After insert in ct:"<<endl;
            //print ct
            for (std::vector<correlation_entry>::iterator iter=ct.begin(); iter!=ct.end(); iter++)
            {
                cout<<"In CT Producer is: "<<hex<<iter->GetProducerPC()<<" ConsumerPC is "<<iter->GetConsumerPC()<<" template is: "<<iter->GetDisass()<<endl;
            }
#endif
        }

        //step 3, insert to ppw
#ifdef DEBUG
        cout<<"STEP 3:  update PPW"<<endl;
#endif
        for (std::map<IntPtr, uint64_t>::iterator it_ppw = ppw.begin(); it_ppw!=ppw.end(); it_ppw++)
        {
            if (( it_ppw->first == dynins->eip) && (it_ppw->second == dynins->target_reg[dynins->num_target_reg-1]))
            {
                already_in_ppw = true;
#ifdef DEBUG
                cout<<"In ppw already have one "<<hex<<dynins->eip<<" target reg is "<<hex<<dynins->target_reg[dynins->num_target_reg-1] <<endl;
#endif
            }
        }
        if (!already_in_ppw)
        {
#ifndef INFINITE_PPW
            if(ppw.size() >= potential_producer_window_size)
            {
                std::map<IntPtr, uint64_t>::iterator it_delete = ppw.end();
                it_delete--;
                //TODO: ppw replacement algorithm should be updated
#ifdef DEBUG
                cout<<"PPW is full, erasing: "<<it_delete->first<<endl;;
#endif
                ppw.erase(it_delete);
            }
#endif
            //if( dynins->instruction->getDisassembly().find("push") == string::npos &&
            //        dynins->instruction->getDisassembly().find("pop") == string::npos &&
            //        dynins->target_reg[dynins->num_target_reg-1] > 0xfffff) /* if target reg is small than 0xfffff, not an base address for others, throw it*/
            if(dynins->target_reg[dynins->num_target_reg-1] > 0xfffff) /* if target reg is small than 0xfffff, not an base address for others, throw it*/
            {
                //the most right reg is the target reg loaded from memory
#ifdef DEBUG
                cout<<"Inserting to ppw: "<<hex<<dynins->eip<<" target reg is "<<hex<<dynins->target_reg[dynins->num_target_reg-1] <<endl;
#endif
                pair<std::map<IntPtr, uint64_t>::iterator, bool> ppw_return=ppw.insert(std::make_pair(dynins->eip, dynins->target_reg[dynins->num_target_reg-1]));
                //if current ip already in the PPW, ppw_return = false and update it.
                if(ppw_return.second == false)
                {
                    ppw.erase(dynins->eip);
                    ppw.insert(std::make_pair(dynins->eip, dynins->target_reg[dynins->num_target_reg-1]));
                }
#ifdef DEBUG
                for (std::map<IntPtr, uint64_t>::iterator it_ppw = ppw.begin(); it_ppw!=ppw.end(); it_ppw++)
                {
                    cout<<"After insert in ppw, PC: "<<hex<<it_ppw->first<<" target reg is "<<hex<<it_ppw->second<<endl;
                }
#endif
            }
        }

        //step 4, lookup CT to get the next prefetch address
        //PC as producer to get potential consumer
#ifdef DEBUG
        cout<<"STEP 4:  get the prefetch address from CT"<<endl;
        //cout<<"Current CT is: "<<endl;
        ////print ct
        //for (std::vector<correlation_entry>::iterator iter=ct.begin(); iter!=ct.end(); iter++)
        //{
        //    cout<<"In CT Producer is: "<<hex<<iter->GetProducerPC()<<" ConsumerPC is "<<iter->GetConsumerPC()<<" template is: "<<iter->GetDisass()<<endl;
        //}
#endif
        std::vector<IntPtr> addresses;

        //for (uint32_t k=0; k<correlation_table_size; k++)
        for (std::vector<correlation_entry>::iterator iter=ct.begin(); iter!=ct.end(); iter++)
        {
            //To get the offset for every comsumer inst, used to compute the prefetch address
            if (iter->GetProducerPC() == dynins->eip)
            {
#ifdef DEBUG
                cout<<"Found prefetch address for 0x"<<hex<<dynins->eip<<endl;
#endif
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
                        stringstream offset(sub_str);
                        offset>>hex>>inst_offset_cn;
#ifdef DEBUG
                        cout<<sub_str<<endl;
                        cout<<dec<<inst_offset<<endl;
#endif

                        if(index6!=string::npos)
                            inst_offset_cn=-inst_offset_cn;

                    }
                }

#ifdef DEBUG
                cout<<" inst_offset_cn 0x"<<hex<<inst_offset_cn<<endl;
#endif
                //get consumer PC, may be multiple
                IntPtr prefetch_address = dynins->target_reg[dynins->num_target_reg-1] + inst_offset_cn;
                bool address_found = false;
                // But stay within the page if requested
                if ((!stop_at_page || ((prefetch_address & PAGE_MASK) == (current_address & PAGE_MASK))) && addresses.size() < num_prefetches)
                {
                    if (prefetch_address > 0xfffff && !only_count_lds)
                    {
                        #ifdef DEBUG
                        //cout<<"For PC "<<hex<<dynins->eip<<" current access address is "<<current_address + offset<<" Prefetch address  is "<<hex<<prefetch_address<<endl;
                        #endif
                        // when to use my idea, we must send the actual offset of the cache block to cache,
                        // not only the cache block address, cause we need the actual data to determine the next address
                        //if (prefetch_address % cache_block_size)
                        //    prefetch_address = prefetch_address-(prefetch_address % cache_block_size);
                        if (prefetch_address!=current_address)
                        {
                            for(std::vector<IntPtr>::iterator it = addresses.begin(); it != addresses.end(); ++it)
                            {
                                //check if this address already have been inserted
                                //if (prefetch_address == *it)
                                if ((prefetch_address-(prefetch_address % cache_block_size)) == (*it-(*it % cache_block_size)) )
                                    address_found=true;
                            }
                            if (!address_found)
                            {
                                addresses.push_back(prefetch_address);
#ifdef DEBUG
                                cout<<"for address: "<<current_address + offset<<" address push_back is "<<hex<<prefetch_address<<" After align: "<<prefetch_address-(prefetch_address % cache_block_size)<<endl;
#endif
                            }
#ifdef DEBUG
                            //cout<<"After align to cache block size,current address is "<<hex<<current_address + offset<<" real Prefetch address  is "<<prefetch_address<<endl;
#endif
                        }
                    }
                }
            }
        }
        return addresses;
    }
    std::vector<IntPtr> empty_addresses;
    return empty_addresses;
}
