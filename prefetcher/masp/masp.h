// masp.h
#ifndef MASP_H
#define MASP_H

#include <cstdint>
#include <optional>

#include "address.h"
#include "champsim.h"
#include "modules.h"
#include "msl/lru_table.h"

struct masp : public champsim::modules::prefetcher {
  struct prediction_entry {
    champsim::address ip{};                          // instruction pointer
    champsim::page_number prev_page{};               // last TLB-miss virtual page
    champsim::page_number::difference_type stride{}; // stride between pages

    auto index() const {
      using namespace champsim::data::data_literals;
      return ip.slice_upper<2_b>();
    }
    auto tag() const {
      using namespace champsim::data::data_literals;
      return ip.slice_upper<2_b>();
    }
  };

  constexpr static std::size_t TRACKER_SETS = 256;
  constexpr static std::size_t TRACKER_WAYS = 4;

  // LRU table for prediction entries
  champsim::msl::lru_table<prediction_entry> table{TRACKER_SETS, TRACKER_WAYS};

public:
  using champsim::modules::prefetcher::prefetcher;

  // Called on every cache access; treats cold first touch of a page as a TLB miss
  uint32_t prefetcher_cache_operate(champsim::address addr,
                                    champsim::address ip,
                                    uint8_t cache_hit,
                                    bool useful_prefetch,
                                    access_type type,
                                    uint32_t metadata_in);

  void prefetcher_cycle_operate();

  uint32_t prefetcher_cache_fill(champsim::address addr,
                                 long set,
                                 long way,
                                 uint8_t prefetch,
                                 champsim::address evicted_addr,
                                 uint32_t metadata_in);
};

#endif // MASP_H