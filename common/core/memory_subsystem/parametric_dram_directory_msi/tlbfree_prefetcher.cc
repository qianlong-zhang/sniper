#include "tlbfree_prefetcher.h"
#include "simulator.h"
#include "config.hpp"
#include "instruction.h"
#include <string>

#include <cstdlib>

const IntPtr PAGE_SIZE = 4096;
const IntPtr PAGE_MASK = ~(PAGE_SIZE-1);
//#define INFINITE_CT
//#define INFINITE_PPW
#if 0
   extern Lock iolock;
#  include "core_manager.h"
#  include "simulator.h"
#  define LOCKED(...) { ScopedLock sl(iolock); fflush(stderr); __VA_ARGS__; fflush(stderr); }
#  define LOGID() fprintf(stderr, "[    ] %2u%c  %-25s@%3u: ", \
                    Sim()->getCoreManager()->getCurrentCoreID(), \
                    Sim()->getCoreManager()->amiUserThread() ? '^' : '_', \
                     __FUNCTION__, __LINE__ \
                  );
#  define MYLOG(...) LOCKED(LOGID(); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n");)
#  define DUMPDATA(data_buf, data_length) { for(UInt32 i = 0; i < data_length; ++i) fprintf(stderr, "%02x ", data_buf[i]); }

#else
#  define MYLOG(...) {}
#endif

TLBFreePrefetcher::TLBFreePrefetcher(String configName, core_id_t _core_id, UInt32 _shared_cores)
   : core_id(_core_id)
   , shared_cores(_shared_cores)
   , n_flows(Sim()->getCfg()->getIntArray("perf_model/" + configName + "/prefetcher/tlbfree/flows", core_id))
   , flows_per_core(Sim()->getCfg()->getBoolArray("perf_model/" + configName + "/prefetcher/tlbfree/flows_per_core", core_id))
   , num_prefetches(Sim()->getCfg()->getIntArray("perf_model/" + configName + "/prefetcher/tlbfree/num_prefetches", core_id))
   , stop_at_page(Sim()->getCfg()->getBoolArray("perf_model/" + configName + "/prefetcher/tlbfree/stop_at_page_boundary", core_id))
   , cache_block_size(Sim()->getCfg()->getIntArray("perf_model/" + configName + "/cache_block_size", core_id))
   , only_count_lds(Sim()->getCfg()->getBoolArray("perf_model/" + configName + "/prefetcher/tlbfree/only_count_lds", core_id))
   , n_flow_next(0)
   , m_prev_address(flows_per_core ? shared_cores : 1)
   , potential_producer_window(flows_per_core ? shared_cores : 1)
   , correlation_table(flows_per_core ? shared_cores : 1)
   , prefetch_request_queue(flows_per_core ? shared_cores : 1)
   , prefetch_buffer(flows_per_core ? shared_cores : 1)
   , potential_producer_window_size(Sim()->getCfg()->getIntArray("perf_model/" + configName + "/prefetcher/tlbfree/potential_producer_window_size", core_id))  /*those param are only used to limit the size of the queue.*/
   , correlation_table_size(Sim()->getCfg()->getIntArray("perf_model/" + configName + "/prefetcher/tlbfree/correlation_table_size", core_id))
   , prefetch_request_queue_size(Sim()->getCfg()->getIntArray("perf_model/" + configName + "/prefetcher/tlbfree/prefetch_request_queue_size", core_id))
   , prefetch_buffer_size(Sim()->getCfg()->getIntArray("perf_model/" + configName + "/prefetcher/tlbfree/prefetch_buffer_size", core_id))
{
   for(UInt32 idx = 0; idx < (flows_per_core ? shared_cores : 1); ++idx)
   	{
      m_prev_address.at(idx).resize(n_flows);

#if 0
    /* For tlbfree prefetcher, create PPW/CT/PRQ/PB */
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
TLBFreePrefetcher::getNextAddress(IntPtr current_address, UInt32 offset, core_id_t _core_id, DynamicInstruction *dynins, UInt64 *pointer_loads,IntPtr target_reg)
{
    int32_t ppw_found=false;
    //bool ct_found=false;
	IntPtr PR = 0;
    bool already_in_ppw = false;
    bool already_in_ct = false;

    //if dynins = 0, that means, this memory access is send by doPrefetch(), so we not prefetch for them again
    if (dynins!=0 && target_reg != 0)
    {
        IntPtr CN = dynins->eip;
        //only deal with memory read, whose target reg is not empty

        //String inst_template = dynins->instruction->getDisassembly();

        /* The outest vector is entry number */
        std::map<IntPtr, uint64_t>   &ppw = potential_producer_window.at(flows_per_core ? _core_id - core_id : 0);
        std::map<IntPtr, uint64_t> temp_ppw;
        std::vector <correlation_entry>      &ct = correlation_table.at(flows_per_core ? _core_id - core_id : 0);
        correlation_entry temp_ct(0,0,"");
        //std::unordered_map<IntPtr, IntPtr>   &prq = prefetch_request_queue.at(flows_per_core ? _core_id - core_id : 0);
        //Cache*                               &pb = prefetch_buffer.at(flows_per_core ? _core_id - core_id : 0);

        MYLOG(" ");
        MYLOG(" ");
        MYLOG("In function: %s,  current_address is: 0x%lx" ,__func__, current_address);
        MYLOG("After add offset: current_address is: 0x%lx", current_address+offset);
        MYLOG("diss is: %s, eip is: 0x%lx", itostr( dynins->instruction->getDisassembly()).c_str(), dynins->eip);

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
                MYLOG("sub_str: %s", itostr(sub_str).c_str());
                MYLOG("inst_offset: %d", inst_offset);
            }
        }

        MYLOG("The real base reg is: 0x%lx", current_address+offset-inst_offset);
        MYLOG("STEP 1:  find producer in PPW");
        //step 1: find producer in PPW, the base address is the whole address(current_address+offset) - inst_offset
        for (std::map<IntPtr, uint64_t>::iterator it = ppw.begin(); it!=ppw.end(); it++)
        {
            if(it->second == current_address+offset-inst_offset)
            {
                //if ppw has more than one hit, multihit
                //assert(ppw_found==false);
                ppw_found=true;
                //record in temp_ppw to get the current inst's producer, which is the nearest one smaller then current one
                MYLOG(" Found in PPW");
                temp_ppw.insert(std::make_pair(it->first, it->second));
            }
        }

        //step 2 put PR/CN/TMPL into CT, hit in PPW
        MYLOG("STEP 2:  if hit in PPW, update CT");
        if( ppw_found == true )
        {
            if (pointer_loads)
                (*pointer_loads)++;
            MYLOG("temp_ppw size is %ld", temp_ppw.size());
            for (std::map<IntPtr, uint64_t>::iterator it = temp_ppw.begin(); it!=temp_ppw.end(); it++)
            {
                MYLOG("In temp_ppw PC is: 0x%lx,  TargetValue is: 0x%lx", it->first, it->second);
            }

            // find the nearest one as the PR, and the PR should smaller than current inst
            // the smaller one has higher priority, find the first smaller one from the end
            for (std::map<IntPtr, uint64_t>::reverse_iterator rit = temp_ppw.rbegin(); rit!=temp_ppw.rend(); rit++)
            {
                if (rit->first <= dynins->eip)
                {
                    PR = rit->first;
                    MYLOG("For inst: 0x%lx  Producer is: 0x%lx TargetValue is: 0x%lx", dynins->eip, PR, rit->second);
                    break;
                }
            }
            //if not found, find a nearest bigger one from the begain
            if (!PR)
            {
                for (std::map<IntPtr, uint64_t>::iterator it = temp_ppw.begin(); it!=temp_ppw.end(); it++)
                {
                    MYLOG("In temp_ppw PC is:0x%lx  it->second is:0x%lx ", it->first, it->second);
                    if (it->first > dynins->eip)
                    {
                        PR = it->first;
                        MYLOG("For inst: 0x%lx,  Producer is: 0x%lx, TargetValue is: 0x%lx", dynins->eip, PR, it->second);
                        break;
                    }
                }
            }
            temp_ppw.clear();

            temp_ct.SetCT(PR, CN, inst_temp);
#ifndef INFINITE_CT
           while (ct.size()>=correlation_table_size)
            {
               vector<correlation_entry>::iterator k=ct.begin();
               ct.erase(k);
            }
#endif


            for (std::vector<correlation_entry>::iterator iter1=ct.begin(); iter1!=ct.end(); iter1++)
            {
                if((iter1->GetProducerPC() == PR)&&(iter1->GetConsumerPC() == CN))
                {
                    already_in_ct = true;
                    MYLOG("In ct already have one PR is: 0x%lx, CN is 0x%lx", PR, CN);
                }
            }
            if (!already_in_ct &&
                    dynins->instruction->getDisassembly().find("push") == string::npos &&
                    dynins->instruction->getDisassembly().find("pop") == string::npos)
            {
                ct.push_back(temp_ct);
            }
            MYLOG("After insert in ct:");
            //print ct
            for (std::vector<correlation_entry>::iterator iter=ct.begin(); iter!=ct.end(); iter++)
            {
                MYLOG("In CT Producer is:0x%lx ConsumerPC is: 0x%lx  template is:0x%s", iter->GetProducerPC(), iter->GetConsumerPC(), itostr(iter->GetDisass()).c_str());
            }
        }

        //step 3, insert to ppw
        MYLOG("STEP 3:  update PPW");
        for (std::map<IntPtr, uint64_t>::iterator it_ppw = ppw.begin(); it_ppw!=ppw.end(); it_ppw++)
        {
            if (( it_ppw->first == dynins->eip) && (it_ppw->second == target_reg))
            {
                already_in_ppw = true;
                MYLOG("In ppw already have one eip: 0x%lx  target reg is:0x%lx ", dynins->eip, target_reg );
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
                MYLOG("PPW is full, erasing: 0x%lx", it_delete->first);;
                ppw.erase(it_delete);
            }
#endif
            //if( dynins->instruction->getDisassembly().find("push") == string::npos &&
            //        dynins->instruction->getDisassembly().find("pop") == string::npos &&
            //        dynins->target_reg[dynins->num_target_reg-1] > 0xfffff) /* if target reg is small than 0xfffff, not an base address for others, throw it*/
            if(target_reg> 0xfffff) /* if target reg is small than 0xfffff, not an base address for others, throw it*/
            {
                //the most right reg is the target reg loaded from memory
                MYLOG("Inserting to ppw eip: 0x%lx,   target reg is 0x%lx", dynins->eip,target_reg);
                pair<std::map<IntPtr, uint64_t>::iterator, bool> ppw_return=ppw.insert(std::make_pair(dynins->eip, target_reg));
                //if current ip already in the PPW, ppw_return = false and update it.
                if(ppw_return.second == false)
                {
                    ppw.erase(dynins->eip);
                    ppw.insert(std::make_pair(dynins->eip, target_reg));
                }
                for (std::map<IntPtr, uint64_t>::iterator it_ppw = ppw.begin(); it_ppw!=ppw.end(); it_ppw++)
                {
                    MYLOG("After insert in ppw, PC: 0x%lx target reg is 0x%lx", it_ppw->first,it_ppw->second);
                }
            }
        }

        //step 4, lookup CT to get the next prefetch address
        //PC as producer to get potential consumer
        MYLOG("STEP 4:  get the prefetch address from CT");
        std::vector<IntPtr> addresses;

        //for (uint32_t k=0; k<correlation_table_size; k++)
        for (std::vector<correlation_entry>::iterator iter=ct.begin(); iter!=ct.end(); iter++)
        {
            //To get the offset for every comsumer inst, used to compute the prefetch address
            if (iter->GetProducerPC() == dynins->eip)
            {
                MYLOG("Found prefetch address for 0x%lx", dynins->eip);
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
                        MYLOG("sub_str: %s", itostr(sub_str).c_str());
                        MYLOG("inst_offset: %d", inst_offset);

                        if(index6!=string::npos)
                            inst_offset_cn=-inst_offset_cn;

                    }
                }

                MYLOG(" inst_offset_cn 0x%x", inst_offset_cn);
                //get consumer PC, may be multiple
                IntPtr prefetch_address = target_reg + inst_offset_cn;
                bool address_found = false;
                // But stay within the page if requested
                if ((!stop_at_page || ((prefetch_address & PAGE_MASK) == (current_address & PAGE_MASK))) && addresses.size() < num_prefetches)
                {
                    if (prefetch_address > 0xfffff && !only_count_lds)
                    {
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
                                MYLOG("producer is: 0x%lx,  consumer is: 0x%lx ,for eip:0x%lx address push_back is 0x%lx After align:0x%lx ",
                                        dynins->eip,
                                        iter->GetConsumerPC(),
                                        current_address + offset,
                                        prefetch_address, prefetch_address-(prefetch_address % cache_block_size));
                            }
                            MYLOG("After align to cache block size,current address is 0x%lx real Prefetch address  is 0x%lx", current_address + offset, prefetch_address);
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
