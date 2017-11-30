#ifndef __SPECULATIVE_SNOOP_PREFETCHER_H
#define __SPECULATIVE_SNOOP_PREFETCHER_H

#include "prefetcher.h"

class SpeculatvieSnoopPrefetcher : public Prefetcher
{
   public:
      SpeculatvieSnoopPrefetcher(String configName, core_id_t core_id, UInt32 shared_cores);
      virtual std::vector<IntPtr> getNextAddress(IntPtr current_address, core_id_t core_id);

   private:
      const core_id_t core_id;
      const core_id_t req_master_id;
      const UInt32 shared_cores;
      const UInt32 n_flows;
      const bool flows_per_core;
      const UInt32 num_prefetches;
      const bool stop_at_page;
      UInt32 n_flow_next;
      std::vector<std::vector<IntPtr> > m_prev_address;
};

#endif //__SPECULATIVE_SNOOP_PREFETCHER_H
