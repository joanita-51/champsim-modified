#include <iostream>
#include <map>
#include <vector>
#include <cstdint>
#include <iomanip>

// Number of interleaved processes/traces
constexpr int      MAX_CONTEXTS               = 2;

// Switch interval in cycles (≈ total_cycles / MAX_CONTEXTS)
constexpr uint64_t CONTEXT_SWITCH_INTERVAL    = 33'000'000ULL;

using Address = uint64_t;

// Per-process PQ state
struct PQState {
    std::vector<Address> addresses;
    bool                 has_prefetch_state = false;
};

uint64_t                      global_cycle    = 0;
uint8_t                       current_ctx     = 0;
std::map<uint8_t, PQState>    pq_map;
bool                          prefetcher_cold = true;

// Save the prefetcher state for the current context
void save_prefetcher_state(uint8_t ctx_id) {
    pq_map[ctx_id].has_prefetch_state = true;
    std::cout << "💾 Saved prefetcher state for ctx " << unsigned(ctx_id) << "\n";
}

// Restore the prefetcher state for the new context
void restore_prefetcher_state(uint8_t ctx_id) {
    if (pq_map[ctx_id].has_prefetch_state) {
        prefetcher_cold = false;
        std::cout << "✅ Restored prefetcher state for ctx " << unsigned(ctx_id) << "\n";
    } else {
        prefetcher_cold = true;
        std::cout << "⚠️  Cold-start: No prefetcher state for ctx " << unsigned(ctx_id) << "\n";
    }
}

// This signature matches what you'll declare in ChampSim:
//   extern void simulate_context_switch(uint64_t cycle);
void simulate_context_switch(uint64_t cycle, bool verbose=true) {
    if (cycle != 0 && cycle % CONTEXT_SWITCH_INTERVAL == 0) {
        std::cout << "\n⏱️  Context Switch at cycle "
                  << std::setw(6) << cycle
                  << ": " << unsigned(current_ctx) << "\n";

        save_prefetcher_state(current_ctx);
        current_ctx = (current_ctx + 1) % MAX_CONTEXTS;
        restore_prefetcher_state(current_ctx);
        std::cout << "Current context" << unsigned(current_ctx) << "\n";
    }
}

// // Simulates prefetching activity per cycle for the current context
// void simulate_prefetch_activity(uint8_t ctx_id, uint64_t cycle) {
//     Address fake_addr = (ctx_id << 12) | (cycle & 0xFFF);
//     pq_map[ctx_id].addresses.push_back(fake_addr);

//     if (prefetcher_cold) {
//         std::cout << "🚫 Cold-prefetching penalty for ctx " << unsigned(ctx_id)
//                   << " at cycle " << cycle << "\n";
//     } else {
//         std::cout << "📦 Prefetched addr 0x" << std::hex << fake_addr
//                   << std::dec << " for ctx " << unsigned(ctx_id)
//                   << " at cycle " << cycle << "\n";
//     }
// }

// int main() {
//     for (global_cycle = 0; global_cycle < SIM_CYCLES; ++global_cycle) {
//         simulate_context_switch(global_cycle);
//         simulate_prefetch_activity(current_ctx, global_cycle);
//     }
//     return 0;
// }
