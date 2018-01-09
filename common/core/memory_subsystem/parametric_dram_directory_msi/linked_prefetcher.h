#ifndef __LINKED_PREFETCHER_H
#define __LINKED_PREFETCHER_H

#include "prefetcher.h"
#include "cache.h"
#include <unordered_map>

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

class correlation_entry {
    private:
        uint64_t ProducerPC;
        uint64_t ConsumerPC;
        DynamicInstruction *Dynins;
    public:
        correlation_entry(uint64_t pr, uint64_t cn, DynamicInstruction * dynins)
        {
            ProducerPC = pr;
            ConsumerPC = cn;
            Dynins = dynins;
        }
        ~correlation_entry()
        {
        }

        void SetCT(uint64_t Producer, uint64_t Consumer, DynamicInstruction *dynins)
        {
            ProducerPC = Producer;
            ConsumerPC = Consumer;
            Dynins = dynins;
        }
        uint64_t GetProducerPC()
        {
            return ProducerPC;
        }
        uint64_t GetConsumerPC()
        {
            return ConsumerPC;
        }
        DynamicInstruction* GetDynins()
        {
            return Dynins;
        }
};


class LinkedPrefetcher : public Prefetcher
{
   public:
      LinkedPrefetcher(String configName, core_id_t core_id, UInt32 shared_cores);
      virtual std::vector<IntPtr> getNextAddress(IntPtr current_address, core_id_t core_id, DynamicInstruction *dynins);

   private:
      const core_id_t core_id;
      const UInt32 shared_cores;
      const UInt32 n_flows;
      const bool flows_per_core;
      const UInt32 num_prefetches;
      const bool stop_at_page;
      UInt32 n_flow_next;
      vector<vector<IntPtr> > m_prev_address;



      /* The outest vector is coreID number, inner vector index is entry number */
      vector< unordered_map<IntPtr, uint64_t> > potential_producer_window ;        //ProgramCounter, TargetValue
      vector< vector < correlation_entry > > correlation_table;                                //correlation table
	  vector< unordered_map<IntPtr, IntPtr> > prefetch_request_queue;           //ProgramCounter, AddressValue
      vector< Cache*> prefetch_buffer;

	 uint32_t potential_producer_window_size;  //those param are only used to limit the size of the queue.
	 uint32_t correlation_table_size;
	 uint32_t prefetch_request_queue_size;
	 uint32_t prefetch_buffer_size;



};

#endif // __LINKED_PREFETCHER_H
