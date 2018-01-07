#include "linked_prefetcher.h"
#include "simulator.h"
#include "config.hpp"

#include <cstdlib>

const IntPtr PAGE_SIZE = 4096;
const IntPtr PAGE_MASK = ~(PAGE_SIZE-1);

LinkedPrefetcher::LinkedPrefetcher(String configName, core_id_t _core_id, UInt32 _shared_cores, DynamicInstruction *dynins)
   : core_id(_core_id)
   , shared_cores(_shared_cores)
   , n_flows(Sim()->getCfg()->getIntArray("perf_model/" + configName + "/prefetcher/linked/flows", core_id))
   , flows_per_core(Sim()->getCfg()->getBoolArray("perf_model/" + configName + "/prefetcher/linked/flows_per_core", core_id))
   , num_prefetches(Sim()->getCfg()->getIntArray("perf_model/" + configName + "/prefetcher/linked/num_prefetches", core_id))
   , stop_at_page(Sim()->getCfg()->getBoolArray("perf_model/" + configName + "/prefetcher/linked/stop_at_page_boundary", core_id))
   , n_flow_next(0)
   , m_prev_address(flows_per_core ? shared_cores : 1)
   , potential_producer_window_size(Sim()->getCfg()->getIntArray("perf_model/" + configName + "/prefetcher/linked/potential_producer_window_size", core_id))  /*those param are only used to limit the size of the queue.*/
   , correlation_table_size(Sim()->getCfg()->getIntArray("perf_model/" + configName + "/prefetcher/linked/correlation_table_size", core_id))
   , prefetch_request_queue_size(Sim()->getCfg()->getIntArray("perf_model/" + configName + "/prefetcher/linked/prefetch_request_queue_size", core_id))
   , prefetch_buffer_size(Sim()->getCfg()->getIntArray("perf_model/" + configName + "/prefetcher/linked/prefetch_buffer_size", core_id))
{
   for(UInt32 idx = 0; idx < (flows_per_core ? shared_cores : 1); ++idx)
   	{
      m_prev_address.at(idx).resize(n_flows);
	  //potential_producer_window.at(idx).resize(potential_producer_window_size);
	  correlation_table.at(idx).resize(correlation_table_size);
	  //prefetch_request_queue.at(idx).resize(prefetch_request_queue_size);
	  //prefetch_buffer.at(idx).resize(prefetch_buffer_size);
   	
#if 0
    /* For linked prefetcher, create PPW/CT/PRQ/PB */
      prefetch_buffer.at(_core_id) = new Cache(configName,
                    configName,
                    _core_id,
                    1,/* set */
                    prefetch_buffer_size,/* associativity */
                    64,
                    "lru",
                    CacheBase::PR_L1_CACHE,
                    CacheBase::parseAddressHash("mask"),
                    NULL);

#endif
   	}
}

std::vector<IntPtr>
LinkedPrefetcher::getNextAddress(IntPtr current_address, core_id_t _core_id, DynamicInstruction *dynins)
{
    int32_t ppw_found=false;
    bool ct_found=false;
	uint32_t opcode = dynins->instruction;
	IntPtr CN = dynins->eip;
	IntPtr PR = 0;

	String inst_template = dynins->instruction->getDisassembly();

    /* The outest vector is entry number */
   std::unordered_map<IntPtr, IntPtr>   &ppw = potential_producer_window.at(flows_per_core ? _core_id - core_id : 0);
   std::vector <correlation_entry>      &ct = correlation_table.at(flows_per_core ? _core_id - core_id : 0);
   std::unordered_map<IntPtr, IntPtr>   &prq = prefetch_request_queue.at(flows_per_core ? _core_id - core_id : 0);
   Cache*                               &pb = prefetch_buffer.at(flows_per_core ? _core_id - core_id : 0);




   for (std::unordered_map<IntPtr, IntPtr>::iterator it = ppw.begin(); it!=ppw.end(); it++)
   {
       //step 1: find producer in PPW
       if(it->second == current_address)
       {
           //if ppw has more than one hit, multihit
           assert(ppw_found==false);
           PR = it->first;
           ppw_found=true;
       }
   }

    //hit in PPW
	if( ppw_found == true )
   	{
		//step 2 put PR/CN/TMPL into CT
        for (uint32_t j=0; j<correlation_table_size; j++)
        {
            //found empty one
            if(!ct.at(j).GetProducerPC())
            {
                ct.at(j).SetCT(PR, CN, dynins);
                ct_found = true;
                break;
            }
        }
        if (!ct_found)
        {
            //delete one CT entry-the last one, and insert
            assert(ct.at(correlation_table_size-1).GetProducerPC != 0);
            ct.at(correlation_table_size-1).SetCT(PR, CN, dynins);
        }
   	}
    //step 3, insert to ppw
    if(ppw.size() >= potential_producer_window_size)
    {
        ppw.erase(ppw.end());
    }
    //if base address is sp, do NOT enter PPW
    if( dynins->instruction->getDisassembly().find("sp") != string::npos)
    {
	    //the most right reg is the target reg loaded from memory
        ppw.insert(dynins->eip, dynins->target_reg[dynins->num_target_reg-1]);
    }

    //step 4, lookup CT to get the next prefetch address
    //PC as producer to get potential consumer
    std::vector<IntPtr> addresses;

    for (uint32_t k=0; k<correlation_table_size; k++)
    {
        if (ct.at(k).GetProducerPC() == dynins->eip)
        {
			 // compute the offset of load instruction to compute the next prefetch address
			int index1 = ct.at(k).GetDyins()->instruction->getDisassembly().find_first_of("(", 0);
			int inst_offset=0;
			if (index1 !=string::npos )
			{
				string sub_str1 = ct.at(k).GetDyins()->instruction->getDisassembly().substr(0, index1);
				cout<<sub_str1<<endl;
				int index2 = sub_str1.find_last_of(" ");
				cout<<index2<<endl;
				if (index2 !=string::npos )
				{
					string sub_str2 = sub_str1.substr(index2+1, index1-1);
					cout<<sub_str2<<endl;
					stringstream offset(sub_str2);
					offset>>hex>>inst_offset;
					cout<<inst_offset<<endl; 																																												}
			}
	
            //get consumer PC, may be multiple
            IntPtr prefetch_address = ct.at(k).GetConsumerPC() + inst_offset;
            // But stay within the page if requested
            if ((!stop_at_page || ((prefetch_address & PAGE_MASK) == (current_address & PAGE_MASK))) && addresses.size() < num_prefetches)
                addresses.push_back(prefetch_address);
        }
    }

	//print ppw
	 for (std::unordered_map<IntPtr, IntPtr>::iterator it = ppw.begin(); it!=ppw.end(); it++)
	 {
	 	cout<<"In PPW PC is: "<<it->first<<" TargetValue is"<<it->second<<endl;
	 }
	//print ct
	 for (uint32_t j=0; j<correlation_table_size; j++)
	 {
	 	cout<<"In CT Producer is: "<<ct.at(j).GetProducerPC()<<" ConsumerPC is "<<ct.at(j).GetConsumerPC()<<endl;
	 }

   return addresses;
}
