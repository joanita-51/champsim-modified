// masp.cc
#include "masp.h"
#include "cache.h"

uint32_t masp::prefetcher_cache_operate(champsim::address addr,
                                        champsim::address ip,
                                        uint8_t cache_hit,
                                        bool useful_prefetch,
                                        access_type type,
                                        uint32_t metadata_in) {
  // Determine virtual page of the address
  champsim::page_number page{addr};
  champsim::page_number::difference_type delta = 0;

  auto found = table.check_hit({ip, page, delta});

  if (found.has_value()) {
    // Extract old page and last stride
    auto old_page = found->prev_page;
    auto last_stride = found->stride;
    
    // Prefetch using last recorded stride
    if (last_stride != 0) {
      auto pf_page = champsim::page_number{page} + last_stride;
      // Issue prefetch of PTE (modeled as a cache line prefetch)
      prefetch_line(champsim::address{pf_page}, /* to L1 */ true, 0);
    }

    // Compute new stride and prefetch using that
    champsim::page_number::difference_type new_delta = champsim::offset(old_page, page);
    if (new_delta != 0 && new_delta != found->stride) {
      auto pf_page2 = champsim::page_number{page} + new_delta;
      prefetch_line(champsim::address{pf_page2}, /* to L1 */ true, 0);
    }

    delta = new_delta;
  }

  // Update table entry
  table.fill({ip, page, delta});

  return metadata_in;
}

void masp::prefetcher_cycle_operate() {
  // MASP issues prefetches immediately on TLB miss; no cycle-based lookahead
}

uint32_t masp::prefetcher_cache_fill(champsim::address addr,
                                     long set,
                                     long way,
                                     uint8_t prefetch,
                                     champsim::address evicted_addr,
                                     uint32_t metadata_in) {
  return metadata_in;
}
