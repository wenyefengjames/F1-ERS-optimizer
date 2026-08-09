#include <benchmark/benchmark.h>
#include "../include/physics.h"

namespace p = physics;

// Below the taper threshold -- pure Euler stepping the whole way, table lookup
// never engages. Baseline cost of the numerical integration loop itself.
static void BM_EnergyDeployedWithTaper_BelowThreshold(benchmark::State& state) {
    for (auto _ : state) {
        auto result = p::energy_deployed_with_taper(150.0, 200.0, -1, -1, false);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_EnergyDeployedWithTaper_BelowThreshold);

// Crosses the taper threshold mid-run (Hamilton Straight's real entry speed/
// length/SM window) -- exercises the search_taper_table lookup path too.
static void BM_EnergyDeployedWithTaper_CrossesThreshold(benchmark::State& state) {
    for (auto _ : state) {
        auto result = p::energy_deployed_with_taper(243.0, 410.0, 25.0, 325.0, true);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_EnergyDeployedWithTaper_CrossesThreshold);

// The accelerating branch's minimum-deployment-power search -- this is the
// one where step size (MGU_K_step_size) directly trades precision for how
// many times the inner Euler loop reruns. Worth re-running this benchmark
// whenever that step size changes, to see the cost/precision tradeoff in
// concrete numbers rather than just intuition.
static void BM_TimeToReachSpeedOverDistance_Accelerating(benchmark::State& state) {
    for (auto _ : state) {
        auto result = p::time_to_reach_speed_over_distance(200.0, 280.0, 300.0, true, false);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_TimeToReachSpeedOverDistance_Accelerating);

// The decelerating (superclip) branch -- closed-form, no search loop, so this
// is a useful contrast against the accelerating branch's cost above.
static void BM_TimeToReachSpeedOverDistance_Decelerating(benchmark::State& state) {
    for (auto _ : state) {
        auto result = p::time_to_reach_speed_over_distance(280.0, 200.0, 300.0, true, false);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_TimeToReachSpeedOverDistance_Decelerating);
