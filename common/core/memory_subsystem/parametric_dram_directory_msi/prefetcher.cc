#include "prefetcher.h"
#include "simulator.h"
#include "config.hpp"
#include "log.h"
#include "simple_prefetcher.h"
#include "ghb_prefetcher.h"
#include "linkednew_prefetcher.h"
#include "tlbfree_prefetcher.h"
#include "cache_cntlr.h"

Prefetcher* Prefetcher::createPrefetcher(String type, String configName, core_id_t core_id, UInt32 shared_cores, void * cache_cntlr)
{
   if (type == "none")
      return NULL;
   else if (type == "simple")
      return new SimplePrefetcher(configName, core_id, shared_cores, cache_cntlr);
   else if (type == "ghb")
      return new GhbPrefetcher(configName, core_id, cache_cntlr);
   else if (type == "tlbfree")
      return new TLBFreePrefetcher(configName, core_id, shared_cores, cache_cntlr);
   else if (type == "linkednew")
       return new LinkednewPrefetcher(configName, core_id, shared_cores, cache_cntlr);

   LOG_PRINT_ERROR("Invalid prefetcher type %s", type.c_str());
}


