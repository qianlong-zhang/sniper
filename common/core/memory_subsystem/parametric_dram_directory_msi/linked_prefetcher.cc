#include "linked_prefetcher.h"
#include "simulator.h"
#include "config.hpp"

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
   , potential_producer_window_size(Sim()->getCfg()->getIntArray("perf_model/" + configName + "/prefetcher/linked/potential_producer_window_size", core_id));  //those param are only used to limit the size of the queue.
   , correlation_table_size(Sim()->getCfg()->getIntArray("perf_model/" + configName + "/prefetcher/linked/correlation_table_size", core_id));;
   , prefetch_request_queue_size(Sim()->getCfg()->getIntArray("perf_model/" + configName + "/prefetcher/linked/prefetch_request_queue_size", core_id));;
   , prefetch_buffer_size(Sim()->getCfg()->getIntArray("perf_model/" + configName + "/prefetcher/linked/prefetch_buffer_size", core_id));;
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
          prefetch_buffer = new Cache(configName,
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
	IntPtr PR = 0;
	uint32_t opcode = dynins->instruction;
	IntPtr CN = dynins->eip;

   /* The outest vector is coreID: XXX */
   std::unordered_map<IntPtr, IntPtr>  &ppw = potential_producer_window.at(flows_per_core ? _core_id - core_id : 0);					
   std::unordered_multimap<IntPtr, std::unordered_map<IntPtr, std::vector<uint32_t, uint32_t> > > &ct = correlation_table.at(flows_per_core ? _core_id - core_id : 0);	  
   std::unordered_map<IntPtr, IntPtr> &prq = prefetch_request_queue.at(flows_per_core ? _core_id - core_id : 0); 								
   Cache* &pb = prefetch_buffer.at(flows_per_core ? _core_id - core_id : 0);

	if( ppw.count(current_address) )
   	{
		for (std::unordered_map<IntPtr, IntPtr>::iterator ppw_it = ppw.begin();
				ppw_it ! = ppw.end;
				ppw_it ++)
		{
			if (*ppw_it.first == current_address)
				PR = *ppw_it.second;
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
