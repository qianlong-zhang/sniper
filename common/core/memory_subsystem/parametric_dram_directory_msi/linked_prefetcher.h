#ifndef __LINKED_PREFETCHER_H
#define __LINKED_PREFETCHER_H

#include "prefetcher.h"

class LinkedPrefetcher : public Prefetcher
{
   public:
      LinkedPrefetcher(String configName, core_id_t core_id, UInt32 shared_cores, DynamicInstruction *dynins);
      virtual std::vector<IntPtr> getNextAddress(IntPtr current_address, core_id_t core_id);

   private:
      const core_id_t core_id;
      const UInt32 shared_cores;
      const UInt32 n_flows;
      const bool flows_per_core;
      const UInt32 num_prefetches;
      const bool stop_at_page;
      UInt32 n_flow_next;
      std::vector<std::vector<IntPtr> > m_prev_address;


	  int32_t potential_producer_window_size;  //those param are only used to limit the size of the queue.
	  int32_t correlation_table_size;
	  int32_t prefetch_request_queue_size;
	  int32_t prefetch_buffer_size;

	  /* The outest vector is coreID: XXX */
	  std::vector<std::unordered_map<IntPtr, IntPtr> > potential_producer_window;                              //AddressValue, Producer
      std::vector<std::unordered_multimap<IntPtr, String > > > correlation_table;    //Producer, Consumer, Template(Opcode, offset)
	  std::vector<std::unordered_map<IntPtr, IntPtr> > prefetch_request_queue;                                 //ProgramCounter, AddressValue
      std::vector<Cache*> prefetch_buffer;


	  
};

#endif // __LINKED_PREFETCHER_H
