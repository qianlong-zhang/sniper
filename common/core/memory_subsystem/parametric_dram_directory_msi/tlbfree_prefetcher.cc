#include "tlbfree_prefetcher.h"
#include "simulator.h"
#include "config.hpp"
#include "instruction.h"
#include <string>
#include <algorithm>

#include <cstdlib>

const IntPtr PAGE_SIZE = 4096;
const IntPtr PAGE_MASK = ~(PAGE_SIZE-1);
//#define INFINITE_CT
//#define INFINITE_PPW
//#define INFINITE_DEP_CT
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
   , num_flattern_prefetches(Sim()->getCfg()->getIntArray("perf_model/" + configName + "/prefetcher/tlbfree/num_flattern_prefetches", core_id))
   , stop_at_page(Sim()->getCfg()->getBoolArray("perf_model/" + configName + "/prefetcher/tlbfree/stop_at_page_boundary", core_id))
   , cache_block_size(Sim()->getCfg()->getIntArray("perf_model/" + configName + "/cache_block_size", core_id))
   , only_count_lds(Sim()->getCfg()->getBoolArray("perf_model/" + configName + "/prefetcher/tlbfree/only_count_lds", core_id))
   , n_flow_next(0)
   , m_prev_address(flows_per_core ? shared_cores : 1)
   , potential_producer_window(flows_per_core ? shared_cores : 1)
   , tlbfree_ppw(flows_per_core ? shared_cores : 1)
   , correlation_table(flows_per_core ? shared_cores : 1)
   , prefetch_request_queue(flows_per_core ? shared_cores : 1)
   , prefetch_buffer(flows_per_core ? shared_cores : 1)
   , potential_producer_window_size(Sim()->getCfg()->getIntArray("perf_model/" + configName + "/prefetcher/tlbfree/potential_producer_window_size", core_id))  /*those param are only used to limit the size of the queue.*/
   , correlation_table_size(Sim()->getCfg()->getIntArray("perf_model/" + configName + "/prefetcher/tlbfree/correlation_table_size", core_id))
   , correlation_table_dep_size(Sim()->getCfg()->getIntArray("perf_model/" + configName + "/prefetcher/tlbfree/correlation_table_dep_size", core_id))
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

void TLBFreePrefetcher::PushInPrefetchList(IntPtr current_address, IntPtr prefetch_address, std::vector<IntPtr> *prefetch_list, UInt32 max_prefetches)
{
    bool address_found =false;
    if ((!stop_at_page || ((prefetch_address & PAGE_MASK) == (current_address & PAGE_MASK))) && (*prefetch_list).size() < max_prefetches)
    {
        if (prefetch_address > 0x400000 && !only_count_lds && prefetch_address<0x800000000000)
        {
            // when to use my idea, we must send the actual offset of the cache block to cache,
            // not only the cache block address, cause we need the actual data to determine the next address
            //if (prefetch_address % cache_block_size)
            //    prefetch_address = prefetch_address-(prefetch_address % cache_block_size);
            if (prefetch_address!=current_address)
            {
                for(std::vector<IntPtr>::iterator it = (*prefetch_list).begin(); it != (*prefetch_list).end(); ++it)
                {
                    //check if this address already have been inserted
                    //if (prefetch_address == *it)
                    if ((prefetch_address-(prefetch_address % cache_block_size)) == (*it-(*it % cache_block_size)) )
                        address_found=true;
                }
                if (!address_found)
                {
                    (*prefetch_list).push_back(prefetch_address);
                    //MYLOG("Pushing back  current_address :0x%lx address push_back is 0x%lx After align:0x%lx ",
                    //        current_address ,
                    //        prefetch_address,
                    //        prefetch_address-(prefetch_address % cache_block_size));
                }
                //MYLOG("After align to cache block size,current address is 0x%lx real Prefetch address  is 0x%lx", current_address, prefetch_address);
            }
        }
    }
}

int32_t TLBFreePrefetcher::DisassGetOffset(std::string inst_disass)
{

    std::string inst_temp = inst_disass;

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
    return inst_offset;
}

std::vector<IntPtr>
TLBFreePrefetcher::getNextAddress(IntPtr current_address, UInt32 offset, core_id_t _core_id, DynamicInstruction *dynins, UInt64 *pointer_loads,IntPtr target_reg)
{
    int32_t ppw_found=false;
	IntPtr PR = 0;
    bool already_in_ppw = false;
    bool already_in_ct = false;
    bool already_in_dep_ct = false;
    int32_t inst_offset= DisassGetOffset(dynins->instruction->getDisassembly().c_str());

    //dynins=0 means memory access is send by doPrefetch(), so we not prefetch for them again
    //dir=0 means write
    if (dynins!=0 && target_reg != 0 && !dynins->memory_info[0].dir)
    {
        IntPtr CN = dynins->eip;
        UInt32 data_size = 0;

        //String inst_template = dynins->instruction->getDisassembly();

        /* The outest vector is entry number */
        std::vector<potential_producer_entry>   &ppw = tlbfree_ppw.at(flows_per_core ? _core_id - core_id : 0);
        std::vector<potential_producer_entry> temp_ppw;
        std::vector <correlation_entry>      &ct = correlation_table.at(flows_per_core ? _core_id - core_id : 0);
        correlation_entry temp_ct(0,0,"",0);
        //std::unordered_map<IntPtr, IntPtr>   &prq = prefetch_request_queue.at(flows_per_core ? _core_id - core_id : 0);
        //Cache*                               &pb = prefetch_buffer.at(flows_per_core ? _core_id - core_id : 0);

        MYLOG(" ");
        MYLOG(" ");
        MYLOG(" memory dir: %d", dynins->memory_info[0].dir);
        if(dynins->num_memory>1)
            MYLOG(" memory_info 2:%d",dynins->memory_info[1].dir );

        MYLOG("In function: %s,  current_address is: 0x%lx" ,__func__, current_address);
        MYLOG("After add offset: current_address is: 0x%lx", current_address+offset);
        MYLOG("diss is: %s, eip is: 0x%lx", itostr( dynins->instruction->getDisassembly()).c_str(), dynins->eip);

        MYLOG("The real base reg is: 0x%lx", current_address+offset-inst_offset);
        MYLOG("STEP 1:  find producer in PPW");

        //step 1: find producer in PPW, the base address is the whole address(current_address+offset) - inst_offset
        for (std::vector<potential_producer_entry>::reverse_iterator it = ppw.rbegin(); it!=ppw.rend(); it++)
        {
            if(it->GetTargetValue() == current_address+offset-inst_offset)
            {
                ppw_found=true;
                MYLOG(" Found in PPW");
                //record in temp_ppw to get the current inst's producer, which is the nearest one smaller then current one
                PR = it->GetProducerPC();
                data_size = it->GetDataSize();
                //lookup from back, the first found one is the newest, so it is the producer
                break;
            }
        }

        //step 2: hit in PPW, put PR/CN/TMPL into CT
        MYLOG("STEP 2:  if hit in PPW, update CT");
        if( ppw_found == true )
        {
            if (pointer_loads)
                (*pointer_loads)++;

            temp_ct.SetCT(PR, CN, dynins->instruction->getDisassembly().c_str(), data_size);
            temp_ct.SetConsumerOffset(inst_offset);

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
            already_in_ct = false;

            //insert to DepList
            MYLOG("Inserting to DepList");
            for (std::vector<correlation_entry>::iterator iter1=ct.begin(); iter1!=ct.end(); iter1++)
            {
                MYLOG("DepList size for iter1: 0x%lx, is %ld", iter1->GetProducerPC(), iter1->DepList.size());
                //if not empty, insert to DepList end
                if(iter1->DepList.size())
                {
                    std::vector<correlation_entry>::reverse_iterator end= iter1->DepList.rbegin();
                    MYLOG("end->GetProducerPC is 0x%lx, end->GetConsumerPC is 0x%lx", end->GetProducerPC(), end->GetConsumerPC());
                    if(end->GetConsumerPC()==PR)
                    {
                        MYLOG("PR and end->GetConsumerPC is 0x%lx", end->GetConsumerPC());
                        for (std::vector<correlation_entry>::iterator iter2=iter1->DepList.begin(); iter2!=iter1->DepList.end(); iter2++)
                        {
                            MYLOG("dealing with PR: 0x%lx, CN: 0x%lx", iter2->GetProducerPC(), iter2->GetProducerPC());
                            if((iter2->GetProducerPC() == PR)&&(iter2->GetConsumerPC() == CN))
                            {
                                already_in_dep_ct = true;
                                MYLOG("In dep ct already have one PR is: 0x%lx, CN is 0x%lx", PR, CN);
                            }
                        }
                        if (!already_in_dep_ct &&
                                dynins->instruction->getDisassembly().find("push") == string::npos &&
                                dynins->instruction->getDisassembly().find("pop") == string::npos)
                        {
                            MYLOG("DepList pushing back: PR 0x%lx, CN 0x%lx, Diss: %s, size:%d", temp_ct.GetProducerPC(),temp_ct.GetConsumerPC(), itostr(temp_ct.GetDisass()).c_str(), temp_ct.GetDataSize() );
                            iter1->DepList.push_back(temp_ct);
                        }
                    }
                    already_in_dep_ct = false;
                }
                else//DepList is zero, insert the first to DepList
                {
                    if(iter1->GetConsumerPC()==PR)
                    {
                        MYLOG("DepList pushing back: PR 0x%lx, CN 0x%lx, Diss: %s, size:%d", temp_ct.GetProducerPC(),temp_ct.GetConsumerPC(), itostr(temp_ct.GetDisass()).c_str(), temp_ct.GetDataSize() );
                        iter1->DepList.push_back(temp_ct);
                    }
                }

#ifndef INFINITE_DEP_CT
                while (iter1->DepList.size()>=correlation_table_dep_size)
                {
                    vector<correlation_entry>::iterator j=iter1->DepList.begin();
                    iter1->DepList.erase(j);
                }
#endif
            }
            //print ct
            MYLOG("After insert in ct:");
            for (std::vector<correlation_entry>::iterator iter=ct.begin(); iter!=ct.end(); iter++)
            {
                //MYLOG("In CT Producer is:0x%lx ConsumerPC is: 0x%lx  template is:%s, consumer offset:%d, data size is %d", iter->GetProducerPC(), iter->GetConsumerPC(), itostr(iter->GetDisass()).c_str(),  iter->GetConsumerOffset(), iter->GetDataSize());
                MYLOG("In CT Producer is:0x%lx ConsumerPC is: 0x%lx  template is:%s", iter->GetProducerPC(), iter->GetConsumerPC(), itostr(iter->GetDisass()).c_str());
                if(iter->DepList.size())
                {
                    for (std::vector<correlation_entry>::iterator iter3=iter->DepList.begin(); iter3!=iter->DepList.end(); iter3++)
                    {
                        MYLOG("\t\tIn DepList CT Producer is:0x%lx ConsumerPC is: 0x%lx  template is:%s, consumer offset:%d, data size is %d", iter3->GetProducerPC(), iter3->GetConsumerPC(), itostr(iter3->GetDisass()).c_str(), iter3->GetConsumerOffset(), iter3->GetDataSize());
                    }
                }
            }
        }

        //step 3, insert to ppw
        MYLOG("STEP 3:  update PPW");
#ifndef INFINITE_PPW
        if(ppw.size() >= potential_producer_window_size)
        {
            std::vector<potential_producer_entry>::iterator it_delete = ppw.begin();
            //TODO: ppw replacement algorithm should be updated
            MYLOG("PPW is full, erasing: 0x%lx", it_delete->GetProducerPC());;
            ppw.erase(it_delete);
        }
#endif

        potential_producer_entry ppw_entry(dynins->eip, target_reg, (dynins->num_memory>1) ? (dynins->memory_info[1].size) : (dynins->memory_info[0].size));
        std::vector<potential_producer_entry>::iterator temp_it = ppw.begin();
        for (std::vector<potential_producer_entry>::iterator it_ppw = ppw.begin(); it_ppw!=ppw.end(); it_ppw++)
        {
            if (( it_ppw->GetProducerPC() == dynins->eip))
            {
                temp_it = it_ppw;
                already_in_ppw = true;
                MYLOG("In ppw already have one eip: 0x%lx  target reg is:0x%lx ", dynins->eip, target_reg );
            }
        }
        if (!already_in_ppw)
        {
            if(target_reg> 0xfffff) /* if target reg is small than 0xfffff, not an base address for others, throw it*/
            {
                //the most right reg is the target reg loaded from memory
                MYLOG("Inserting to ppw eip: 0x%lx,   target reg is 0x%lx", dynins->eip,target_reg);
                ppw.push_back(ppw_entry);
            }
        }
        else
        {
            MYLOG("updating ppw, deleting 0x%lx, target_reg is 0x%lx", temp_it->GetProducerPC(), temp_it->GetTargetValue());
            ppw.erase(temp_it);
            ppw.push_back(ppw_entry);
        }
        //std::sort(ppw.begin(), ppw.end(), comp);
        //print ppw
        for (std::vector<potential_producer_entry>::iterator it_ppw = ppw.begin(); it_ppw!=ppw.end(); it_ppw++)
        {
            //MYLOG("After insert in ppw, PC: 0x%lx target reg is 0x%lx", it_ppw->GetProducerPC(),it_ppw->GetTargetValue());
            MYLOG("After insert in ppw, PC: 0x%lx", it_ppw->GetProducerPC());
        }
        already_in_ppw=false;

        //step 4, lookup CT to get the next prefetch address
        //PC as producer to get potential consumer
        MYLOG("STEP 4:  get the prefetch address from CT");
        std::vector<IntPtr> addresses;

        //for (uint32_t k=0; k<correlation_table_size; k++)
        for (std::vector<correlation_entry>::iterator iter=ct.begin(); iter!=ct.end(); iter++)
        {
            IntPtr temp_temp_target_reg=0;
            if (iter->GetProducerPC() == dynins->eip)
            {
                MYLOG("Pushing back PR is 0x%lx, CN is 0x%lx,current address is 0x%lx,  Prefetch address is 0x%lx", iter->GetProducerPC(), iter->GetConsumerPC(),current_address + offset , target_reg + iter->GetConsumerOffset());
                PushInPrefetchList(current_address+offset, target_reg + iter->GetConsumerOffset(), &addresses, num_flattern_prefetches);

                for(int32_t j = iter->GetDataSize()-1; j >= 0; --j)
                {
                    temp_temp_target_reg = (temp_temp_target_reg << 8) | reinterpret_cast<Byte *>(current_address+offset)[j];
                }
                MYLOG("target_reg is 0x%lx, temp_temp_target_reg is 0x%lx, num_memory is %d, first memory address is 0x%lx, size is%d, GetDataSize() is %d", target_reg, temp_temp_target_reg, dynins->num_memory, dynins->memory_info[0].addr, dynins->memory_info[0].size, iter->GetDataSize());
            }
        }
        //print addresses
        for(std::vector<IntPtr>::iterator it = addresses.begin(); it != addresses.end(); ++it)
        {
            MYLOG("After push back in prefetch address: 0x%lx", *it);
        }
#if 0
        /****************************************/
        /****************************************/
        /****************************************/
        //get dependency access address to prefetch
        if(num_prefetches - num_flattern_prefetches > 0)
        {
            IntPtr temp_target_reg=0;
            IntPtr last_prefetch_address=0;
            for (std::vector<correlation_entry>::iterator iter=ct.begin(); iter!=ct.end(); iter++)
            {
                //To get the offset for every comsumer inst, used to compute the prefetch address
                if ((iter->GetProducerPC() == dynins->eip) && (iter->DepList.size()>0))
                {
                    last_prefetch_address = target_reg + iter->GetConsumerOffset();
                    PushInPrefetchList(current_address+offset, last_prefetch_address, &addresses, num_prefetches);

                    if (!stop_at_page || ((last_prefetch_address & PAGE_MASK) == (current_address & PAGE_MASK)) )
                    {
                        MYLOG("\t\tDepList PR: 0x%lx, CN: 0x%lx, Getting data from 0x%lx, size:%d",iter->GetProducerPC(), iter->GetConsumerPC(), last_prefetch_address, iter->GetDataSize());
                        for(int32_t j = iter->GetDataSize()-1; j >= 0; --j)
                        {
                            temp_target_reg = (temp_target_reg << 8) | reinterpret_cast<Byte *>(last_prefetch_address)[j];
                        }
                        MYLOG("\t\tDepList data is 0x%lx", temp_target_reg);
                    }
                    for (std::vector<correlation_entry>::iterator iter4=iter->DepList.begin(); iter4!=iter->DepList.end(); iter4++)
                    {
                        PushInPrefetchList(last_prefetch_address, temp_target_reg + iter4->GetConsumerOffset(), &addresses, num_prefetches);
                        last_prefetch_address = temp_target_reg + iter4->GetConsumerOffset();
                        if (!stop_at_page || ((last_prefetch_address & PAGE_MASK) == (current_address & PAGE_MASK)) )
                        {
                            MYLOG("\t\tDepList PR: 0x%lx, CN: 0x%lx, Getting data from 0x%lx, size:%d",iter4->GetProducerPC(), iter4->GetConsumerPC(), last_prefetch_address, iter4->GetDataSize());
                            for(int32_t j = iter4->GetDataSize()-1; j >= 0; --j)
                            {
                                temp_target_reg = (temp_target_reg << 8) | reinterpret_cast<Byte *>(last_prefetch_address)[j];
                            }
                            MYLOG("\t\tDepList data is 0x%lx", temp_target_reg);
                        }
                    }
                }
            }
        }
#endif
        return addresses;
    }
    std::vector<IntPtr> empty_addresses;
    return empty_addresses;
}
