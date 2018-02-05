#ifndef PREFETCHER_H
#define PREFETCHER_H

#include "fixed_types.h"
#include "dynamic_instruction.h"

#include <vector>
class correlation_entry {
    private:
        uint64_t ProducerPC;
        uint64_t ConsumerPC;
        std::string disass;
    public:
        correlation_entry(uint64_t pr, uint64_t cn, std::string dis)
        {
            ProducerPC = pr;
            ConsumerPC = cn;
            disass = dis;
        }
        ~correlation_entry()
        {
        }

        void SetCT(uint64_t Producer, uint64_t Consumer, std::string dis)
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
        std::string GetDisass()
        {
            return disass;
        }
};

class Prefetcher
{
   public:
      static Prefetcher* createPrefetcher(String type, String configName, core_id_t core_id, UInt32 shared_cores);

      virtual std::vector<IntPtr> getNextAddress(IntPtr current_address, UInt32 offset, core_id_t core_id, DynamicInstruction *dynins, UInt64 *pointer_loads) = 0;
};

#endif // PREFETCHER_H
