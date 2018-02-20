#ifndef PREFETCHER_H
#define PREFETCHER_H

#include "fixed_types.h"
#include "dynamic_instruction.h"

#include <vector>
class correlation_entry {
    private:
        uint64_t ProducerPC;

        uint64_t ConsumerPC;
        std::string ConsumerDisass;
        UInt32 ConsumerDataSize; //TODO:this shouold be producer's datasize
        int32_t ConsumerOffset;
    public:
        std::vector <correlation_entry> DepList;
        correlation_entry(uint64_t pr, uint64_t cn, std::string dis, UInt32 size)
        {
            ProducerPC = pr;
            ConsumerPC = cn;
            ConsumerDisass = dis;
            ConsumerDataSize = size;
        }
        ~correlation_entry()
        {
        }

        void SetCT(uint64_t Producer, uint64_t Consumer, std::string dis, UInt32 size)
        {
            ProducerPC = Producer;

            ConsumerPC = Consumer;
            ConsumerDisass = dis;
            ConsumerDataSize = size;
            ConsumerOffset = 0;
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
            return ConsumerDisass;
        }
        UInt32 GetDataSize()
        {
            return ConsumerDataSize;
        }
        int32_t  GetConsumerOffset()
        {
            return ConsumerOffset;
        }


        void SetConsumerOffset(uint32_t off)
        {
            ConsumerOffset = off;
        }
};

class potential_producer_entry{
    private:
        IntPtr ProducerPC;
        IntPtr TargetValue;
        UInt32 DataSize; //Producer data size
    public:
        potential_producer_entry(IntPtr pc, IntPtr target_reg, UInt32 size)
        {
            ProducerPC = pc;
            TargetValue = target_reg;
            DataSize=size;
        }
        ~potential_producer_entry()
        {
        }

        IntPtr GetProducerPC() const
        {
            return ProducerPC;
        }
        uint64_t GetTargetValue()
        {
            return TargetValue;
        }
        UInt32 GetDataSize()
        {
            return DataSize;
        }
};
#if 0
class prefetch_entry{
    private:
        UInt32 offset;
        UInt32 data_size; //consumer data size
    public:
        prefetch_entry(UInt32 off, UInt32 size)
        {
            offset = off;
            data_size = size;
        }
        ~prefetch_entry()
        {
        }

        UInt32 GetOffset()
        {
            return offset;
        }
        UInt32 GetDataSize()
        {
            return data_size;
        }
};
#endif

class Prefetcher
{
   public:
      static Prefetcher* createPrefetcher(String type, String configName, core_id_t core_id, UInt32 shared_cores);

      virtual std::vector<IntPtr> getNextAddress(IntPtr current_address, UInt32 offset, core_id_t core_id, DynamicInstruction *dynins, UInt64 *pointer_loads, UInt64 *pointer_stores, IntPtr target_reg) = 0;
};

#endif // PREFETCHER_H
