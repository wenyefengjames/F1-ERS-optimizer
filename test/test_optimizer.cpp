#include <gtest/gtest.h>
#include "../include/optimizer.h"
#include <set>

// Optimizer's constructor eagerly allocates the full DP table (`table`/`choice`
// each end up with circuit.size()*battery_buckets^2*harvest_buckets entries --
// low tens of millions). index_helper() is pure index arithmetic that never
// reads or writes either vector, so every test below shares these two
// instances (one per race_mode, since harvest_buckets depends on it) instead
// of paying that allocation cost once per test case.
static Optimizer opt(false, false);       // qualifying mode: harvest_buckets = 6.0/0.05 + 1 = 121
static Optimizer race_opt(true, false);   // race mode: harvest_buckets = 8.5/0.05 + 1 = 171

// The real segment count, queried from a live Track rather than hardcoded --
// index_helper's formula multiplies both the e and h terms by circuit.size(),
// so every expected value below depends on it. Hardcoding it as a literal is
// exactly what broke these tests once the track grew from 16 to 22 segments;
// deriving it here keeps them correct regardless of future track changes.
static const int N = Track().size();

// =========================================================================
// Hand-computed flattening: with bucket_size=0.05, battery_buckets=81, the
// multiplier for each dimension should be the product of the sizes of every
// dimension nested inside it:
//   b -> 1
//   i -> battery_buckets                = 81
//   e -> N * battery_buckets            = N * 81
//   h -> N * battery_buckets^2          = N * 81 * 81
// =========================================================================
struct IndexHelperCheck { int i; double b, e, h; int expected_index; };

class CheckIndexHelper : public ::testing::TestWithParam<IndexHelperCheck> {};

TEST_P(CheckIndexHelper, MatchesHandComputedFlattening) {
    IndexHelperCheck c = GetParam();
    EXPECT_EQ(opt.index_helper(c.i, c.b, c.e, c.h), c.expected_index);
}

INSTANTIATE_TEST_SUITE_P(VariousCases, CheckIndexHelper, ::testing::Values(
    IndexHelperCheck{0, 0.0, 0.0, 0.0, 0},                          // all-zero baseline
    IndexHelperCheck{1, 0.0, 0.0, 0.0, 81},                         // i alone -> i * battery_buckets
    IndexHelperCheck{0, 2.0, 0.0, 0.0, 40},                         // b alone -> b_bucket = round(2.0/0.05)
    IndexHelperCheck{0, 0.0, 2.0, 0.0, 40 * N * 81},                // e alone -> 40 * N * battery_buckets
    IndexHelperCheck{0, 0.0, 0.0, 2.0, 40 * N * 81 * 81},           // h alone -> 40 * N * battery_buckets^2
    // combined, to catch any cross-term bug a single-dimension case would miss:
    // 20*(N*81*81) (h) + 10*(N*81) (e) + 3*81 (i) + 20 (b)
    IndexHelperCheck{3, 1.0, 0.5, 1.0, 20 * N * 81 * 81 + 10 * N * 81 + 3 * 81 + 20}
));

// =========================================================================
// std::round is round-half-away-from-zero: 0.024 MJ is bucket 0 (0.024/0.05
// = 0.48), 0.026 MJ is bucket 1 (0.026/0.05 = 0.52) -- straddling that .5
// boundary is the case most likely to expose an off-by-one in the rounding,
// not just a value comfortably inside a bucket.
// =========================================================================
TEST(OptimizerIndexHelperTest, RoundsToNearestBucket) {
    EXPECT_EQ(opt.index_helper(0, 0.024, 0.0, 0.0), opt.index_helper(0, 0.0, 0.0, 0.0));
    EXPECT_EQ(opt.index_helper(0, 0.026, 0.0, 0.0), opt.index_helper(0, 0.05, 0.0, 0.0));
}

// =========================================================================
// The largest valid (i, b, e, h) tuple should land exactly on the last valid
// slot of the table Optimizer's constructor allocates
// (circuit.size()*battery_buckets^2*harvest_buckets - 1) -- checked in both
// modes since harvest_buckets (and so the valid range of h) depends on
// race_mode, even though the formula's multipliers themselves don't.
// =========================================================================
TEST(OptimizerIndexHelperTest, MaxValidStateStaysWithinTableBounds) {
    // qualifying: harvest_buckets = 121 -> max h_bucket = 120
    EXPECT_EQ(opt.index_helper(N - 1, 4.0, 4.0, 6.0), N * 81 * 81 * 121 - 1);

    // race: harvest_buckets = 171 -> max h_bucket = 170
    EXPECT_EQ(race_opt.index_helper(N - 1, 4.0, 4.0, 8.5), N * 81 * 81 * 171 - 1);
}

// =========================================================================
// Sample a modest cross-product of every dimension (all N segments, first
// 4 buckets of b/e/h) and confirm none of the combinations collide -- a
// stronger check than a handful of hand-picked pairs, since a wrong
// multiplier on any one dimension will alias two *different* states onto
// the same index somewhere in a range this size.
// =========================================================================
TEST(OptimizerIndexHelperTest, NoCollisionsAcrossManyStates) {
    std::set<int> seen;

    for (int i = 0; i < N; i++) {
        for (int b_step = 0; b_step < 4; b_step++) {
            for (int e_step = 0; e_step < 4; e_step++) {
                for (int h_step = 0; h_step < 4; h_step++) {
                    int idx = opt.index_helper(i, b_step * 0.05, e_step * 0.05, h_step * 0.05);
                    seen.insert(idx);
                }
            }
        }
    }

    EXPECT_EQ(seen.size(), static_cast<unsigned>(N) * 4u * 4u * 4u);
}
