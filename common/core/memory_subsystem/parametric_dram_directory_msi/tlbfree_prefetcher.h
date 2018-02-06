#ifndef __TLBFREE_PREFETCHER_H
#define __TLBFREE_PREFETCHER_H

#include "prefetcher.h"
#include <unordered_map>
#include "cache_cntlr.h"
#include <map>

using namespace std;

class TLBFreePrefetcher : public Prefetcher
{
   public:
      TLBFreePrefetcher(String configName, core_id_t core_id, UInt32 shared_cores);
      virtual std::vector<IntPtr> getNextAddress(IntPtr current_address, UInt32 offset, core_id_t core_id, DynamicInstruction *dynins, UInt64 *pointer_loads);

   private:
      const core_id_t core_id;
      const UInt32 shared_cores;
      const UInt32 n_flows;
      const bool flows_per_core;
      const UInt32 num_prefetches;
      const bool stop_at_page;
      UInt32 cache_block_size;
      const bool only_count_lds;
      UInt32 n_flow_next;
      vector<vector<IntPtr> > m_prev_address;



      /* The outest vector is coreID number, inner vector index is entry number */
      vector< map<IntPtr, uint64_t> > potential_producer_window ;        //ProgramCounter, TargetValue
      vector< vector < correlation_entry > > correlation_table;                                //correlation table
	  vector< unordered_map<IntPtr, IntPtr> > prefetch_request_queue;           //ProgramCounter, AddressValue
      vector< Cache*> prefetch_buffer;

	 uint32_t potential_producer_window_size;  //those param are only used to limit the size of the queue.
	 uint32_t correlation_table_size;
	 uint32_t prefetch_request_queue_size;
	 uint32_t prefetch_buffer_size;



};

#endif // __TLBFREE_PREFETCHER_H
