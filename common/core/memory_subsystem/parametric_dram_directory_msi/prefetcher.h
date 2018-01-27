#ifndef PREFETCHER_H
#define PREFETCHER_H

#include "fixed_types.h"
#include "dynamic_instruction.h"

#include <vector>

class Prefetcher
{
   public:
      static Prefetcher* createPrefetcher(String type, String configName, core_id_t core_id, UInt32 shared_cores);

      virtual std::vector<IntPtr> getNextAddress(IntPtr current_address, UInt32 offset, core_id_t core_id, DynamicInstruction *dynins, UInt64 *pointer_loads) = 0;
};

#endif // PREFETCHER_H
