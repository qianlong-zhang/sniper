#ifndef __TLBFREE_PREFETCHER_H
#define __TLBFREE_PREFETCHER_H

#include "prefetcher.h"
#include <unordered_map>
#include "cache_cntlr.h"
#include <map>

using namespace std;
#if 0
class ppw_entry {
    private:
        uint64_t PC;
        uint64_t TargetReg;
    public:
        ppw_entry()
        {
            PC = 0;
            TargetReg = 0;
        }
        ~ppw_entry()
        {
        }

        void SetPPW(uint64_t pc, uint64_t target_reg)
        {
            PC = pc;
            TargetReg = target_reg;
        }
        uint64_t GetPC()
        {
            return PC;
        }
        uint64_t GetTargetReg()
        {
            return TargetReg;
        }
};
#endif

#if 0
class correlation_entry {
    private:
        uint64_t ProducerPC;
        uint64_t ConsumerPC;
        string disass;
    public:
        correlation_entry(uint64_t pr, uint64_t cn, string dis)
        {
            ProducerPC = pr;
            ConsumerPC = cn;
            disass = dis;
        }
        ~correlation_entry()
        {
        }

        void SetCT(uint64_t Producer, uint64_t Consumer, string dis)
        {
            ProducerPC = Producer;
            ConsumerPC = Consumer;
            disass = dis;
        }
        uint64_t GetProducerPC()
        {
            return ProducerPC;
        }
        uint64_t GetConsumerPC()
        {
            return ConsumerPC;
        }
        string GetDisass()
        {
            return disass;
        }
};
#endif


class TLBFreePrefetcher : public Prefetcher
{
   public:
      TLBFreePrefetcher(String configName, core_id_t core_id, UInt32 shared_cores);
      /* The first IntPtr is access address, second is offset of next access address
       * which should be added to the data of the access address on the left
       * to get the next address
       */
      virtual std::vector<IntPtr> getNextAddress(IntPtr current_address, UInt32 offset, core_id_t core_id, DynamicInstruction *dynins, UInt64 *pointer_loads);
      IntPtr lookup_ct_get_offset(IntPtr producer, IntPtr consumer, std::vector <correlation_entry>  ct);
      IntPtr lookup_ct_get_consumer(IntPtr producer, std::vector <correlation_entry> ct);

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

#endif //__TLBFREE_PREFETCHER_H
