#include <gtest/gtest.h>
#include "../include/physics.h"
#include <cmath>

namespace p = physics;

struct KECheck {double input_speed_kmh, output_energy_J;};
struct KE2SpeedCheck {double input_energy_J, output_speed_kmh;};

class CheckKEFormula : public ::testing::TestWithParam<KECheck>{};
class CheckKE2SpeedFormula : public ::testing::TestWithParam<KE2SpeedCheck>{};

// Test the correctness of kinetic_energy()
TEST_P(CheckKEFormula, CheckCorrectRandomValues) {
    KECheck k = GetParam();
    EXPECT_NEAR(p::kinetic_energy(k.input_speed_kmh), k.output_energy_J, 5e-4); // 5e-4 should be an error between 3-4 decimal places
}

INSTANTIATE_TEST_SUITE_P(VariousCases, CheckKEFormula, ::testing::Values(
    KECheck{330, 3226666.667},
    KECheck{200, 1185185.185},
    KECheck{100, 296296.2963},
    KECheck{0,0},
    KECheck{-300, 2666666.667},
    KECheck{-330, 3226666.667}
));

// Test the correctness of ke_to_speed()
TEST_P(CheckKE2SpeedFormula, CheckCorrectRandomValues) {
    KE2SpeedCheck k = GetParam();
    EXPECT_NEAR(p::ke_to_speed(k.input_energy_J), k.output_speed_kmh, 5e-4); // 5e-4 should be an error between 3-4 decimal places
}

INSTANTIATE_TEST_SUITE_P(VariousCases, CheckKE2SpeedFormula, ::testing::Values(
    KE2SpeedCheck{3226666.667, 330},
    KE2SpeedCheck{2666666.667, 300},
    KE2SpeedCheck{1185185.185, 200},
    KE2SpeedCheck{296296.2963, 100},
    KE2SpeedCheck{0,0},
    KE2SpeedCheck{-10,-1},
    KE2SpeedCheck{-100,-1},
    KE2SpeedCheck{-0,0}
));

// =========================================================================
// reverse_ke -- verified algebraically against kinetic_energy: adding
// energy_J to the car's current KE and converting back to speed.
// energy = 124416 J is chosen so that 2*energy/MASS_KG = 324 = 18^2 m^2/s^2,
// which keeps the resulting speeds exact square roots instead of ugly
// unverifiable decimals.
// =========================================================================
struct ReverseKECheck { double initial_speed_kmh, energy_J, output_speed_kmh; };

class CheckReverseKEFormula : public ::testing::TestWithParam<ReverseKECheck> {};

TEST_P(CheckReverseKEFormula, CheckCorrectRandomValues) {
    ReverseKECheck k = GetParam();
    EXPECT_NEAR(p::reverse_ke(k.initial_speed_kmh, k.energy_J), k.output_speed_kmh, 1e-4);
}

INSTANTIATE_TEST_SUITE_P(VariousCases, CheckReverseKEFormula, ::testing::Values(
    ReverseKECheck{0, 124416, 64.8},              // 0 -> 18 m/s -> 64.8 km/h
    ReverseKECheck{64.8, 124416, 91.6410388417},   // 18 -> 18*sqrt(2) m/s
    ReverseKECheck{64.8, -124416, 0}               // 18 -> 0 m/s (all added KE removed again)
));

// No validity check like ke_to_speed's -- removing more energy than the car
// currently has drives the sqrt argument negative, silently producing NaN.
TEST(ReverseKeEdgeCase, ReturnsNanWhenRemovingMoreEnergyThanCarHas) {
    double result = p::reverse_ke(64.8, -200000.0);
    EXPECT_TRUE(std::isnan(result));
}

// =========================================================================
// work_done_with_drag -- transcendental (pow/exp), so instead of hand-typing
// a decimal constant, verify it via boundaries, monotonicity, and the two
// invariants that tie it to the other drag formulas -- same idea as the
// checks in physics_check.cpp, translated into GoogleTest.
// =========================================================================
TEST(WorkDoneWithDragProperties, ZeroAtZeroDistance) {
    EXPECT_NEAR(p::work_done_with_drag(300.0, 150.0, 0.0), 0.0, 1e-3);
}

TEST(WorkDoneWithDragProperties, IncreasesWithDistance) {
    double e_100m = p::work_done_with_drag(300.0, 150.0, 100.0);
    double e_200m = p::work_done_with_drag(300.0, 150.0, 200.0);
    EXPECT_GT(e_200m, e_100m);
}

TEST(WorkDoneWithDragProperties, IncreasesWithPower) {
    double e_200kw = p::work_done_with_drag(200.0, 150.0, 100.0);
    double e_300kw = p::work_done_with_drag(300.0, 150.0, 100.0);
    EXPECT_GT(e_300kw, e_200kw);
}

// At zero engine power the only thing happening is drag -- the formula
// should collapse to exactly the negative of coasting_energy_loss.
TEST(WorkDoneWithDragProperties, ZeroPowerMatchesNegativeCoastingLoss) {
    double vi = 150.0, x = 100.0;
    EXPECT_NEAR(p::work_done_with_drag(0.0, vi, x), -p::coasting_energy_loss(vi, x), 1e-3);
}

// As distance -> infinity the car settles at terminal velocity a = (P/k)^(1/3),
// so the KE gained should approach kinetic_energy(terminal_speed) - kinetic_energy(vi).
TEST(WorkDoneWithDragProperties, ApproachesTerminalVelocityKeAtLargeDistance) {
    double power_kW = 300.0, vi = 150.0;
    double terminal_speed_kmh = std::pow(power_kW * 1000.0 / p::DRAG_K, 1.0 / 3.0) * 3.6;
    double expected = p::kinetic_energy(terminal_speed_kmh) - p::kinetic_energy(vi);
    EXPECT_NEAR(p::work_done_with_drag(power_kW, vi, 1e7), expected, 100.0);
}

// =========================================================================
// required_power / distance_to_recharge -- both are inverses of
// work_done_with_drag, so round-tripping through it is a far more reliable
// check than a hand-typed constant would be. Same pattern as
// physics_check.cpp (the distance_to_recharge one is specifically the check
// that would have caught the (-mass/3*k) parenthesization bug).
// =========================================================================
TEST(RequiredPowerProperties, RecoversOriginalPower) {
    double vi = 150.0, x = 100.0, power_kW = 300.0;
    double energy_target = p::work_done_with_drag(power_kW, vi, x);
    double required_power_W = p::required_power(vi, energy_target, x);
    EXPECT_NEAR(required_power_W / 1000.0, power_kW, 1e-2);
}

TEST(DistanceToRechargeProperties, RecoversOriginalDistance) {
    double vi = 150.0, x = 100.0, power_kW = 300.0;
    double energy_target = p::work_done_with_drag(power_kW, vi, x);
    double recovered_distance = p::distance_to_recharge(vi, energy_target, power_kW);
    EXPECT_NEAR(recovered_distance, x, 1e-2);
}

// If the requested energy implies a target speed at or beyond this power's
// terminal velocity, there's no finite distance that reaches it -- the log's
// argument goes negative and the function silently returns NaN.
TEST(DistanceToRechargeEdgeCase, ReturnsNanWhenTargetExceedsTerminalVelocity) {
    double vi = 150.0, power_kW = 300.0;
    double unreachable_target_energy = p::kinetic_energy(1000.0) - p::kinetic_energy(vi);
    double result = p::distance_to_recharge(vi, unreachable_target_energy, power_kW);
    EXPECT_TRUE(std::isnan(result));
}

// =========================================================================
// coasting_energy_loss
// =========================================================================
TEST(CoastingEnergyLossProperties, ZeroAtZeroDistance) {
    EXPECT_NEAR(p::coasting_energy_loss(150.0, 0.0), 0.0, 1e-3);
}

TEST(CoastingEnergyLossProperties, IncreasesWithDistance) {
    double loss_100m = p::coasting_energy_loss(150.0, 100.0);
    double loss_1000m = p::coasting_energy_loss(150.0, 1000.0);
    EXPECT_GT(loss_1000m, loss_100m);
}

TEST(CoastingEnergyLossProperties, NeverExceedsInitialKineticEnergy) {
    double vi = 150.0;
    double ke_vi = p::kinetic_energy(vi);
    double loss_far = p::coasting_energy_loss(vi, 1e7);
    EXPECT_LE(loss_far, ke_vi + 1e-6);
    EXPECT_NEAR(loss_far, ke_vi, 100.0); // approaches it over a long enough coast
}

// =========================================================================
// time_to_reach_velocity -- same reasoning as work_done_with_drag: verify
// via boundary + monotonicity, not a hand-typed constant.
// =========================================================================
TEST(TimeToReachVelocityProperties, ZeroWhenTargetEqualsInitial) {
    EXPECT_NEAR(p::time_to_reach_velocity(150.0, 150.0, 300.0), 0.0, 1e-6);
}

// The second target here is deliberately close to this power's terminal
// velocity (terminal speed ~= 305.7 km/h for 300kW) rather than an arbitrary
// round number -- the closer the target gets to the asymptote, the more time
// it should take, which this also exercises.
TEST(TimeToReachVelocityProperties, IncreasesWithTargetSpeed) {
    double t_to_150 = p::time_to_reach_velocity(150.0, 100.0, 300.0);
    double t_to_290 = p::time_to_reach_velocity(290.0, 100.0, 300.0);
    EXPECT_GT(t_to_150, 0.0);
    EXPECT_GT(t_to_290, t_to_150);
}

TEST(TimeToReachVelocityProperties, HigherPowerTakesLessTimeForSameTarget) {
    double t_300kw = p::time_to_reach_velocity(250.0, 100.0, 300.0);
    double t_400kw = p::time_to_reach_velocity(250.0, 100.0, 400.0);
    EXPECT_GT(t_300kw, t_400kw);
}

// =========================================================================
// taper_curve -- piecewise-linear, hand-computable exactly.
// =========================================================================
struct TaperCheck { double speed_kmh; bool mom; double expected_power_kW; };

class CheckTaperCurve : public ::testing::TestWithParam<TaperCheck> {};

TEST_P(CheckTaperCurve, MatchesPiecewiseLinearFormula) {
    TaperCheck t = GetParam();
    EXPECT_NEAR(p::taper_curve(t.speed_kmh, t.mom), t.expected_power_kW, 1e-3);
}

INSTANTIATE_TEST_SUITE_P(VariousCases, CheckTaperCurve, ::testing::Values(
    TaperCheck{0, false, 350},
    TaperCheck{290, false, 350},     // exactly at the no-MOM breakpoint
    TaperCheck{291, false, 344.6154},
    // Documents a slight mismatch with the "hits zero at 355" spec in
    // CLAUDE.md: the hardcoded -5.38 slope leaves 0.3kW of residual power
    // at 355 km/h rather than exactly 0 (350/65 = 5.3846..., not 5.38).
    TaperCheck{355, false, 0},  // When the curve should hit 0
    TaperCheck{356, false, 0},  // Cap the output non-negative
    TaperCheck{360, false, 0},

    TaperCheck{0, true, 350},
    TaperCheck{337, true, 350},      // exactly at the MOM breakpoint
    TaperCheck{338, true, 330.5556},
    TaperCheck{355, true, 0},   // When the curve should hit 0
    TaperCheck{360, true, 0}    // Cap the output non-negative
));

// No floor at 0 -- an unrealistically high speed drives power negative.
TEST(TaperCurveEdgeCase, InvalidInputsThrowNegative1) {
    EXPECT_NEAR(p::taper_curve(-5.0, false), -1.0, 1e-3);
    EXPECT_NEAR(p::taper_curve(-50.0, false), -1.0, 1e-3);
    EXPECT_NEAR(p::taper_curve(-0.05, false), -1.0, 1e-3);
}

// =========================================================================
// braking_harvest -- linear, hand-computable exactly. Speed deltas are
// chosen as multiples of BRAKING_DECEL*3.6 (194.04) so the division comes
// out clean instead of needing a long-division-derived decimal.
// =========================================================================
struct BrakingHarvestCheck { double current_speed_kmh, target_speed_kmh, expected_energy_J; };

class CheckBrakingHarvest : public ::testing::TestWithParam<BrakingHarvestCheck> {};

TEST_P(CheckBrakingHarvest, MatchesLinearFormula) {
    BrakingHarvestCheck b = GetParam();
    EXPECT_NEAR(p::braking_harvest(b.current_speed_kmh, b.target_speed_kmh), b.expected_energy_J, 1e-3);
}

INSTANTIATE_TEST_SUITE_P(VariousCases, CheckBrakingHarvest, ::testing::Values(
    BrakingHarvestCheck{294.04, 100, 350000},
    BrakingHarvestCheck{488.08, 100, 700000},
    BrakingHarvestCheck{197.02, 100, 175000},
    BrakingHarvestCheck{100, 100, 0}
));

// Invalid inputs should output -1
// Invalid inputs: current_speed < 0, current_speed < target_speed, target_speed < 0
TEST(BrakingHarvestEdgeCase, InvalidInputsThrowNegative1) {
    EXPECT_NEAR(p::braking_harvest(-100.0, 50.0), -1.0, 1e-3);
    EXPECT_NEAR(p::braking_harvest(100.0, -294.04), -1.0, 1e-3);
    EXPECT_NEAR(p::braking_harvest(100.0, -294.04), -1.0, 1e-3);
}

// =========================================================================
// coasting_harvest -- flat rate * time, no branches.
// =========================================================================
struct CoastingHarvestCheck { double time_s, expected_energy_J; };

class CheckCoastingHarvest : public ::testing::TestWithParam<CoastingHarvestCheck> {};

TEST_P(CheckCoastingHarvest, MatchesLinearFormula) {
    CoastingHarvestCheck c = GetParam();
    EXPECT_NEAR(p::coasting_harvest(c.time_s), c.expected_energy_J, 1e-3);
}

INSTANTIATE_TEST_SUITE_P(VariousCases, CheckCoastingHarvest, ::testing::Values(
    CoastingHarvestCheck{0, 0},
    CoastingHarvestCheck{1, 350000},
    CoastingHarvestCheck{2.5, 875000}
));

TEST(CoastingHarvestEdgeCases, InvalidInputsThrowNegative1){
    EXPECT_NEAR(p::coasting_harvest(-0.5), -1.0, 1e-3);
    EXPECT_NEAR(p::coasting_harvest(-12.0), -1.0, 1e-3);
}

// =========================================================================
// superclipping -- clip_rate clamped at MGU_K on the upper end only.
// =========================================================================
struct SuperclippingCheck { double clip_rate_kW, time_s, expected_energy_J; };

class CheckSuperclipping : public ::testing::TestWithParam<SuperclippingCheck> {};

TEST_P(CheckSuperclipping, MatchesClampedLinearFormula) {
    SuperclippingCheck s = GetParam();
    EXPECT_NEAR(p::superclipping(s.clip_rate_kW, s.time_s), s.expected_energy_J, 1e-3);
}

INSTANTIATE_TEST_SUITE_P(VariousCases, CheckSuperclipping, ::testing::Values(
    SuperclippingCheck{300, 2, 600000},
    SuperclippingCheck{350, 1, 350000},    // exactly at the cap
    SuperclippingCheck{400, 2, 700000}     // above the cap, clamped to MGU_K
));

// Invalid inputs should output -1
// Invalid inputs: cliprate < 0, time < 0
TEST(SuperclippingEdgeCase, InvalidInputsThrowNegative1) {
    EXPECT_NEAR(p::superclipping(-100.0, 2.0), -1.0, 1e-3);
    EXPECT_NEAR(p::superclipping(20.0, -0.5), -1.0, 1e-3);
}

// =========================================================================
// partial_throttle_harvest -- recharge_rate = min(MGU_K, (100-throttle)% of ICE).
// =========================================================================
struct PartialThrottleCheck { double throttle_pct, time_s, expected_energy_J; };

class CheckPartialThrottleHarvest : public ::testing::TestWithParam<PartialThrottleCheck> {};

TEST_P(CheckPartialThrottleHarvest, MatchesClampedLinearFormula) {
    PartialThrottleCheck pt = GetParam();
    EXPECT_NEAR(p::partial_throttle_harvest(pt.throttle_pct, pt.time_s), pt.expected_energy_J, 1e-3);
}

INSTANTIATE_TEST_SUITE_P(VariousCases, CheckPartialThrottleHarvest, ::testing::Values(
    PartialThrottleCheck{0, 1, 350000},     // 0% throttle -> would-be 400kW, clamped to MGU_K
    PartialThrottleCheck{50, 1, 200000},
    PartialThrottleCheck{100, 1, 0},        // full throttle -> nothing left to harvest
    PartialThrottleCheck{12.5, 1, 350000},  // exactly at the clamp boundary
    PartialThrottleCheck{10, 1, 350000}     // below the boundary, still clamped
));

// Check that invalid inputs cause an output of -1
// Invalid inputs: throttle% > 100 or < 0, time < 0
TEST(PartialThrottleHarvestEdgeCase, InvalidInputsThrowNegative1) {
    EXPECT_NEAR(p::partial_throttle_harvest(125.0, 1.0), -1.0, 1e-3);
    EXPECT_NEAR(p::partial_throttle_harvest(-10.0, 1.0), -1.0, 1e-3);
    EXPECT_NEAR(p::partial_throttle_harvest(90.0, -1.0), -1.0, 1e-3);
}