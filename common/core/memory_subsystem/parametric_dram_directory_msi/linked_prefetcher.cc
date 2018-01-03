#include "linked_prefetcher.h"
#include "simulator.h"
#include "config.hpp"

#include <cstdlib>

const IntPtr PAGE_SIZE = 4096;
const IntPtr PAGE_MASK = ~(PAGE_SIZE-1);

LinkedPrefetcher::LinkedPrefetcher(String configName, core_id_t _core_id, UInt32 _shared_cores, DynamicInstruction *dynins)
   : core_id(_core_id)
   , shared_cores(_shared_cores)
   , n_flows(Sim()->getCfg()->getIntArray("perf_model/" + configName + "/prefetcher/linked/flows", core_id))
   , flows_per_core(Sim()->getCfg()->getBoolArray("perf_model/" + configName + "/prefetcher/linked/flows_per_core", core_id))
   , num_prefetches(Sim()->getCfg()->getIntArray("perf_model/" + configName + "/prefetcher/linked/num_prefetches", core_id))
   , stop_at_page(Sim()->getCfg()->getBoolArray("perf_model/" + configName + "/prefetcher/linked/stop_at_page_boundary", core_id))
   , n_flow_next(0)
   , m_prev_address(flows_per_core ? shared_cores : 1)
   , potential_producer_window_size(Sim()->getCfg()->getIntArray("perf_model/" + configName + "/prefetcher/linked/potential_producer_window_size", core_id))  /*those param are only used to limit the size of the queue.*/
   , correlation_table_size(Sim()->getCfg()->getIntArray("perf_model/" + configName + "/prefetcher/linked/correlation_table_size", core_id))
   , prefetch_request_queue_size(Sim()->getCfg()->getIntArray("perf_model/" + configName + "/prefetcher/linked/prefetch_request_queue_size", core_id))
   , prefetch_buffer_size(Sim()->getCfg()->getIntArray("perf_model/" + configName + "/prefetcher/linked/prefetch_buffer_size", core_id))
{
   for(UInt32 idx = 0; idx < (flows_per_core ? shared_cores : 1); ++idx)
   	{
      m_prev_address.at(idx).resize(n_flows);
	  //potential_producer_window.at(idx).resize(potential_producer_window_size);
	  //prefetch_request_queue.at(idx).resize(prefetch_request_queue_size);
	  //prefetch_buffer.at(idx).resize(prefetch_buffer_size);
   	}

    /* For linked prefetcher, create PPW/CT/PRQ/PB */
      {
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
      }

}

std::vector<IntPtr>
LinkedPrefetcher::getNextAddress(IntPtr current_address, core_id_t _core_id, DynamicInstruction *dynins)
{
    int32_t ppw_found=false;
    bool ct_found=false;
	uint32_t opcode = dynins->instruction;
	IntPtr CN = dynins->eip;
	IntPtr PR = 0;

	String inst_template = dynins->instruction->getDisassembly();

    /* The outest vector is entry number */
   std::unordered_map<IntPtr, IntPtr>   &ppw = potential_producer_window.at(flows_per_core ? _core_id - core_id : 0);
   std::vector <correlation_entry>      &ct = correlation_table.at(flows_per_core ? _core_id - core_id : 0);
   std::unordered_map<IntPtr, IntPtr>   &prq = prefetch_request_queue.at(flows_per_core ? _core_id - core_id : 0);
   Cache*                               &pb = prefetch_buffer.at(flows_per_core ? _core_id - core_id : 0);

   for (std::unordered_map<IntPtr, IntPtr>::iterator it = ppw.begin(); it!=ppw.end(); it++)
   {
       //step 1: find producer in PPW
       if(it->second == current_address)
       {
           //if ppw has more than one hit, multihit
           assert(ppw_found==-1);
           PR = it->first;
           ppw_found=true;
       }
   }

    //hit in PPW
	if( ppw_found == true )
   	{
		//step 2 put PR/CN/TMPL into CT
        for (uint32_t j=0; j<correlation_table_size; j++)
        {
            //found empty one
            if(ct.at(j).GetProducerPC())
            {
                ct.at(j).SetCT(PR, CN, dynins);
                ct_found = true;
                break;
            }
        }
        if (!ct_found)
        {
            //delete one CT entry-the last one, and insert
            assert(ct.at(correlation_table_size-1).GetProducerPC != 0);
            ct.at(correlation_table_size-1).SetCT(PR, CN, dynins);
        }
   	}
    //step 3, insert to ppw
    if(ppw.size() >= potential_producer_window_size)
    {
        ppw.erase(ppw.end());
    }
    //if base address is sp, do NOT enter PPW
    if( dynins->instruction->getDisassembly().compare("sp") != 0)
    {
        ppw.insert(dynins->eip, dynins->target_reg[dynins->num_target_reg-1]);//the most right reg is the target reg loaded from memory
    }

    //step 4, lookup CT to get the next prefetch address
    //PC as producer to get potential consumer
    std::vector<IntPtr> addresses;
    IntPtr inst_offset = dynins->instruction->getDisassembly().split(' ')
    for (uint32_t k=0; k<correlation_table_size; k++)
    {
        if (ct.at(k).GetProducerPC() == dynins->eip)
        {
            //get consumer PC, may be multiple
            IntPtr prefetch_address = ct.at(k).GetConsumerPC() + inst_offset;
        }
    }





   std::vector<IntPtr> &prev_address = m_prev_address.at(flows_per_core ? _core_id - core_id : 0);

   UInt32 n_flow = n_flow_next;
   IntPtr min_dist = PAGE_SIZE;
   n_flow_next = (n_flow_next + 1) % n_flows; // Round robin replacement

   // Find the nearest address in our list of previous addresses
   for(UInt32 i = 0; i < n_flows; ++i)
   {
      IntPtr dist = abs(current_address - prev_address[i]);
      if (dist < min_dist)
      {
         n_flow = i;
         min_dist = dist;
      }
   }

   // Now, n_flow points to the previous address of the best matching flow
   // (Or, if none matched, the round-robin determined one to replace)

   // linked linear stride prefetcher
   IntPtr stride = current_address - prev_address[n_flow];
   prev_address[n_flow] = current_address;

   std::vector<IntPtr> addresses;
   if (stride != 0)
   {
      for(unsigned int i = 0; i < num_prefetches; ++i)
      {
         IntPtr prefetch_address = current_address + i * stride;
         // But stay within the page if requested
         if (!stop_at_page || ((prefetch_address & PAGE_MASK) == (current_address & PAGE_MASK)))
            addresses.push_back(prefetch_address);
      }
   }

   return addresses;
}
