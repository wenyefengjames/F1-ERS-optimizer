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
    EXPECT_NEAR(p::kinetic_energy(k.input_speed_kmh), k.output_energy_J, 5e-4); // 5e-4 should be the error range between 3-4 decimal places
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

// Validity check will produce -1
TEST(ReverseKeEdgeCase, ReturnsNeg1WhenRemovingMoreEnergyThanCarHas) {
    EXPECT_DOUBLE_EQ(p::reverse_ke(64.8, -200000.0), -1);
}

// =========================================================================
// work_done_with_drag -- transcendental (pow/exp), so instead of hand-typing
// a decimal constant, verify it via boundaries, monotonicity, and the two
// invariants that tie it to the other drag formulas -- same idea as the
// checks in physics_check.cpp, translated into GoogleTest.
// =========================================================================
TEST(WorkDoneWithDragProperties, ZeroAtZeroDistance) {
    EXPECT_NEAR(p::work_done_with_drag(300.0, 150.0, 0.0, false), 0.0, 1e-3);
}

TEST(WorkDoneWithDragProperties, IncreasesWithDistance) {
    double e_100m = p::work_done_with_drag(300.0, 150.0, 100.0, false);
    double e_200m = p::work_done_with_drag(300.0, 150.0, 200.0, false);
    EXPECT_GT(e_200m, e_100m);
}

TEST(WorkDoneWithDragProperties, IncreasesWithPower) {
    double e_200kw = p::work_done_with_drag(200.0, 150.0, 100.0, false);
    double e_300kw = p::work_done_with_drag(300.0, 150.0, 100.0, false);
    EXPECT_GT(e_300kw, e_200kw);
}

// At zero engine power the only thing happening is drag, so the KE change
// should be a pure loss (negative), growing with distance, and -- since
// terminal velocity for zero power is 0 -- approaching -kinetic_energy(vi)
// over a long enough distance. This is the power=0 special case of the
// same asymptote ApproachesTerminalVelocityKeAtLargeDistance checks below
// for nonzero power, so it doesn't need a separate coasting-only formula
// as a reference.
TEST(WorkDoneWithDragProperties, ZeroPowerIsAPureLossApproachingNegativeInitialKe) {
    double vi = 150.0;
    bool sm_on = false;
    double loss_short = p::work_done_with_drag(0.0, vi, 100.0, sm_on);
    double loss_far = p::work_done_with_drag(0.0, vi, 1e7, sm_on);

    EXPECT_LT(loss_short, 0.0);
    EXPECT_LT(loss_far, loss_short); // more distance, more lost
    EXPECT_NEAR(loss_far, -p::kinetic_energy(vi), 100.0);
}

// As distance -> infinity the car settles at terminal velocity a = (P/k)^(1/3),
// so the KE gained should approach kinetic_energy(terminal_speed) - kinetic_energy(vi).
// Uses drag_coeff(sm_on) rather than the old DRAG_K constant now, since
// work_done_with_drag switched to the mode-dependent coefficient -- the
// terminal velocity implied by a given power now depends on sm_on too.
TEST(WorkDoneWithDragProperties, ApproachesTerminalVelocityKeAtLargeDistance) {
    double power_kW = 300.0, vi = 150.0;
    bool sm_on = false;
    double terminal_speed_kmh = std::pow(power_kW * 1000.0 / p::drag_coeff(sm_on), 1.0 / 3.0) * 3.6;
    double expected = p::kinetic_energy(terminal_speed_kmh) - p::kinetic_energy(vi);
    EXPECT_NEAR(p::work_done_with_drag(power_kW, vi, 1e7, sm_on), expected, 100.0);
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
    bool mom = false, sm_on = false;
    double energy_target = p::work_done_with_drag(power_kW, vi, x, sm_on);
    double required_power_W = p::required_power(vi, energy_target, x, mom, sm_on);
    EXPECT_NEAR(required_power_W / 1000.0, power_kW, 1e-2);
}

TEST(DistanceToRechargeProperties, RecoversOriginalDistance) {
    double vi = 150.0, x = 100.0, power_kW = 300.0;
    bool sm_on = false;
    double energy_target = p::work_done_with_drag(power_kW, vi, x, sm_on);
    double recovered_distance = p::distance_to_recharge(vi, energy_target, power_kW, sm_on);
    EXPECT_NEAR(recovered_distance, x, 1e-2);
}

// If the requested energy implies a target speed at or beyond this power's
// terminal velocity, there's no finite distance that reaches it -- the log's
// argument goes negative and the function silently returns NaN.
TEST(DistanceToRechargeEdgeCase, ReturnsNanWhenTargetExceedsTerminalVelocity) {
    double vi = 150.0, power_kW = 300.0;
    double unreachable_target_energy = p::kinetic_energy(1000.0) - p::kinetic_energy(vi);
    double result = p::distance_to_recharge(vi, unreachable_target_energy, power_kW, false);
    EXPECT_TRUE(std::isnan(result));
}

// =========================================================================
// time_to_reach_velocity -- same reasoning as work_done_with_drag: verify
// via boundary + monotonicity, not a hand-typed constant.
// =========================================================================
TEST(TimeToReachVelocityProperties, ZeroWhenTargetEqualsInitial) {
    EXPECT_NEAR(p::time_to_reach_velocity(150.0, 150.0, 300.0, false), 0.0, 1e-6);
}

// The second target here is deliberately close to this power's terminal
// velocity under corner-mode drag (terminal speed ~= 305.7 km/h for 300kW)
// rather than an arbitrary round number -- the closer the target gets to
// the asymptote, the more time it should take, which this also exercises.
TEST(TimeToReachVelocityProperties, IncreasesWithTargetSpeed) {
    double t_to_150 = p::time_to_reach_velocity(150.0, 100.0, 300.0, false);
    double t_to_290 = p::time_to_reach_velocity(290.0, 100.0, 300.0, false);
    EXPECT_GT(t_to_150, 0.0);
    EXPECT_GT(t_to_290, t_to_150);
}

TEST(TimeToReachVelocityProperties, HigherPowerTakesLessTimeForSameTarget) {
    double t_300kw = p::time_to_reach_velocity(250.0, 100.0, 300.0, false);
    double t_400kw = p::time_to_reach_velocity(250.0, 100.0, 400.0, false);
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

// =========================================================================
// Validating the drag equation
// =========================================================================

struct DragCheck {double speed_kmh; bool sm_on; double output;};

class CheckDragFormula : public ::testing::TestWithParam<DragCheck> {};

TEST_P(CheckDragFormula, CorrectValues){
    DragCheck c = GetParam();
    EXPECT_NEAR(p::drag(c.speed_kmh, c.sm_on), c.output, 5e-4);
}

INSTANTIATE_TEST_SUITE_P(VariousCases, CheckDragFormula, ::testing::Values(
    DragCheck{0, false, 0},
    DragCheck{-0.5, false, -1},
    DragCheck{100, false, 685.2816},
    DragCheck{200, false, 2741.1265},
    DragCheck{300, false, 6167.5347},
    DragCheck{0, true, 0},
    DragCheck{-0.5, true, -1},
    DragCheck{100, true, 479.6971},
    DragCheck{200, true, 1918.7886},
    DragCheck{300, true, 4317.2743}
));


TEST(DragFormulaEdgeCases, InvalidInputsThrowNegative1){
    EXPECT_NEAR(p::drag(-0.5, false), -1, 5e-4);
    EXPECT_NEAR(p::drag(-10, true), -1, 5e-4);
}

// =========================================================================
// energy_deployed_with_taper -- numerical (Euler) integration over
// transcendental drag formulas, so verified by property rather than a
// hand-typed constant, same reasoning as the other Tier-B checks above.
// =========================================================================

struct DeployTaperCheck {double speed_kmh; bool mom; double distance, output_kmh;};

// Checking that the output speed is correct before tapering kicks in. Both
// calls must share the same sm_on -- drag_coeff(sm_on) has to match on both
// sides for "matches the untapered closed form" to mean anything.
// sm_start/sm_end = -1 is the sentinel for "no SM anywhere in this run".
TEST(EnergyDeployedWithTaperProperties, MatchesUntaperedBelowThreshold) {
    double vi = 100.0, distance = 5.0;
    bool sm_on = false;
    auto result = p::energy_deployed_with_taper(vi, distance, -1, -1, false);

    double untapered_energy = p::work_done_with_drag(p::MGU_K + p::ICE, vi, distance, sm_on);
    double untapered_speed = p::reverse_ke(vi, untapered_energy);

    EXPECT_NEAR(result.speed_kmh, untapered_speed, untapered_speed * 0.01);
}

// Checking that tapering is working and the output speed should be strictly less than
// The speed without tapering
TEST(EnergyDeployedWithTaperProperties, TaperReducesFinalSpeedAboveThreshold) {
    double vi = 280.0, distance = 300.0;
    bool sm_on = false;
    auto result = p::energy_deployed_with_taper(vi, distance, -1, -1, false);

    double untapered_energy = p::work_done_with_drag(p::MGU_K + p::ICE, vi, distance, sm_on);
    double untapered_speed = p::reverse_ke(vi, untapered_energy);

    EXPECT_LT(result.speed_kmh, untapered_speed);
}

// Checks that the distance covered is at least the requested distance
TEST(EnergyDeployedWithTaperProperties, CoversAtLeastRequestedDistance) {
    double vi = 200.0, distance = 500.0;
    auto result = p::energy_deployed_with_taper(vi, distance, -1, -1, false);

    EXPECT_GE(result.distance_m, distance);
    EXPECT_LT(result.distance_m, distance + (400.0 / 3.6) * p::DELTA_T);
}

// SM active for the whole covered distance should beat no SM at all: SM's
// lower drag coefficient means less energy lost to drag for the same power
// input, so final speed should be strictly higher. Kept comfortably below
// the taper threshold so drag coefficient is the only thing differing
// between the two runs.
TEST(EnergyDeployedWithTaperProperties, SmWindowCoveringWholeRunBeatsNoSm) {
    double vi = 200.0, distance = 300.0;
    auto sm_result = p::energy_deployed_with_taper(vi, distance, 0.0, distance, false);
    auto no_sm_result = p::energy_deployed_with_taper(vi, distance, -1, -1, false);

    EXPECT_GT(sm_result.speed_kmh, no_sm_result.speed_kmh);
}

// distance=0 should lead to every property unchanged, regardless of mom/sm_on --
// parameterized over all 4 combinations since the loop body never runs either way
// (so the specific sm_start/sm_end values chosen for the "SM on" case don't matter).
struct ZeroDistanceCheck { bool mom; bool sm_on; };

class CheckZeroDistanceEdgeCase : public ::testing::TestWithParam<ZeroDistanceCheck> {};

TEST_P(CheckZeroDistanceEdgeCase, ReturnsUnchangedStartingState) {
    ZeroDistanceCheck c = GetParam();
    double vi = 200.0;
    double sm_start = c.sm_on ? 0.0 : -1.0;
    double sm_end = c.sm_on ? 100.0 : -1.0;
    auto result = p::energy_deployed_with_taper(vi, 0.0, sm_start, sm_end, c.mom);

    EXPECT_DOUBLE_EQ(result.speed_kmh, vi);
    EXPECT_DOUBLE_EQ(result.energy_J, 0.0);
    EXPECT_DOUBLE_EQ(result.time_s, 0.0);
    EXPECT_DOUBLE_EQ(result.distance_m, 0.0);
}

INSTANTIATE_TEST_SUITE_P(AllModeCombinations, CheckZeroDistanceEdgeCase, ::testing::Values(
    ZeroDistanceCheck{false, false},
    ZeroDistanceCheck{false, true},
    ZeroDistanceCheck{true, false},
    ZeroDistanceCheck{true, true}
));

// =========================================================================
// drag_coeff -- simple mode switch, hand-computable exactly.
// =========================================================================
TEST(DragCoeffTest, MatchesExpectedValuesForBothModes) {
    EXPECT_NEAR(p::drag_coeff(true), 0.6216875, 1e-7);   // straight mode: 0.5*1.225*1.45*0.7
    EXPECT_NEAR(p::drag_coeff(false), 0.888125, 1e-7);   // corner mode:   0.5*1.225*1.45*1.0
}

TEST(DragCoeffTest, StraightModeHasLowerDragThanCornerMode) {
    EXPECT_LT(p::drag_coeff(true), p::drag_coeff(false));
}

// =========================================================================
// search_taper_table -- the table itself is built by numerical (Euler)
// integration, so its exact contents can't be hand-typed. Verified instead
// by property: searching by the field you searched on should return
// (almost exactly) the query value back; searching by a different field for
// the value just found should agree with it; out-of-range queries should
// return nullopt.
//
// Only (mom=false,sm_on=false), (mom=false,sm_on=true), and (mom=true,sm_on=true)
// are exercised here. taper_table(true, false) returns a reference to a
// destroyed temporary (`return {};` on a `const vector&`-returning function --
// flagged in an earlier review), so calling search_taper_table with that
// combination is undefined behavior, not just "untested."
// =========================================================================
TEST(SearchTaperTableProperties, SearchingBySpeedReturnsMatchingSpeedBack) {
    const auto& table = p::taper_table(false, true);
    ASSERT_GT(table.size(), 2u);
    double query = (table.front().speed_kmh + table.back().speed_kmh) / 2.0;

    auto by_speed = [](const TaperedDeploymentResult& e){ return e.speed_kmh; };
    auto result = p::search_taper_table(false, true, query, by_speed);

    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(result->speed_kmh, query, 1e-6);
}

TEST(SearchTaperTableProperties, SearchingByDifferentFieldsAgreesWithEachOther) {
    const auto& table = p::taper_table(false, true);
    ASSERT_GT(table.size(), 2u);
    double query_speed = (table.front().speed_kmh + table.back().speed_kmh) / 2.0;

    auto by_speed = [](const TaperedDeploymentResult& e){ return e.speed_kmh; };
    auto by_time  = [](const TaperedDeploymentResult& e){ return e.time_s; };

    auto result_by_speed = p::search_taper_table(false, true, query_speed, by_speed);
    ASSERT_TRUE(result_by_speed.has_value());

    auto result_by_time = p::search_taper_table(false, true, result_by_speed->time_s, by_time);
    ASSERT_TRUE(result_by_time.has_value());

    EXPECT_NEAR(result_by_time->speed_kmh, result_by_speed->speed_kmh, 1e-3);
}

TEST(SearchTaperTableProperties, WorksForMomTableToo) {
    const auto& table = p::taper_table(true, true);
    ASSERT_GT(table.size(), 2u);
    double query = (table.front().speed_kmh + table.back().speed_kmh) / 2.0;

    auto by_speed = [](const TaperedDeploymentResult& e){ return e.speed_kmh; };
    auto result = p::search_taper_table(true, true, query, by_speed);

    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(result->speed_kmh, query, 1e-6);
}

TEST(SearchTaperTableEdgeCase, ReturnsNulloptWhenQueryBelowTableStart) {
    auto by_speed = [](const TaperedDeploymentResult& e){ return e.speed_kmh; };
    auto result = p::search_taper_table(false, true, 100.0, by_speed); // below the 290 km/h taper threshold entirely
    EXPECT_FALSE(result.has_value());
}

TEST(SearchTaperTableEdgeCase, ReturnsNulloptWhenQueryAboveTableEnd) {
    auto by_speed = [](const TaperedDeploymentResult& e){ return e.speed_kmh; };
    auto result = p::search_taper_table(false, true, 1000.0, by_speed); // far beyond anything the table could reach
    EXPECT_FALSE(result.has_value());
}

// build_taper_table pushes a sentinel row at the exact starting boundary
// speed before the real simulated steps -- lower_bound finds that sentinel
// row itself for a query landing exactly on it, which is table.begin(), so
// this returns nullopt too rather than the (all-zero) sentinel values.
TEST(SearchTaperTableEdgeCase, ReturnsNulloptExactlyAtTableStart) {
    auto by_speed = [](const TaperedDeploymentResult& e){ return e.speed_kmh; };
    auto result = p::search_taper_table(false, true, 290.0, by_speed);
    EXPECT_FALSE(result.has_value());
}



// =========================================================================
// time_to_reach_speed_over_distance()
// =========================================================================


class TestTimeToReachSpeedOverDistance : public ::testing::TestWithParam<ZeroDistanceCheck>{};

TEST_P(TestTimeToReachSpeedOverDistance, ConsistentOutputWhenNoChangeInVelocity){
    ZeroDistanceCheck c = GetParam();
    auto result = p::time_to_reach_speed_over_distance(180, 180, 100, c.mom, c.sm_on);

    ASSERT_TRUE(result.has_value());
    EXPECT_DOUBLE_EQ(result->time_s, 2);
}

// The function now signals invalid input via std::nullopt rather than a -1 sentinel.
TEST_P(TestTimeToReachSpeedOverDistance, CheckEdgeCases){
    ZeroDistanceCheck c = GetParam();

    EXPECT_FALSE(p::time_to_reach_speed_over_distance(0, 180, 100, c.mom, c.sm_on).has_value());
    EXPECT_FALSE(p::time_to_reach_speed_over_distance(-1, 180, 100, c.mom, c.sm_on).has_value());
    EXPECT_FALSE(p::time_to_reach_speed_over_distance(180, 0, 100, c.mom, c.sm_on).has_value());
    EXPECT_FALSE(p::time_to_reach_speed_over_distance(180, -1, 100, c.mom, c.sm_on).has_value());
    EXPECT_FALSE(p::time_to_reach_speed_over_distance(180, 180, 0, c.mom, c.sm_on).has_value());
    EXPECT_FALSE(p::time_to_reach_speed_over_distance(180, 180, -1, c.mom, c.sm_on).has_value());
}

INSTANTIATE_TEST_SUITE_P(AllModesCombination, TestTimeToReachSpeedOverDistance, testing::Values(
    ZeroDistanceCheck{true, true},
    ZeroDistanceCheck{true, false},
    ZeroDistanceCheck{false, true},
    ZeroDistanceCheck{false, false}
));

// Decelerating branch reuses required_power's exact formula internally, so
// checking against required_power + time_to_reach_velocity directly is a
// more reliable reference than re-deriving the formula by hand -- as long
// as both are called with the same sm_on, since time_to_reach_velocity now
// uses drag_coeff(sm_on) internally instead of the old fixed DRAG_K.
TEST(TimeToReachSpeedOverDistanceProperties, DeceleratingMatchesRequiredPower) {
    double vi = 200.0, vf = 180.0, distance = 300.0;
    bool mom = false, sm_on = false;

    double energy_diff = p::kinetic_energy(vf) - p::kinetic_energy(vi);
    double r_W = p::required_power(vi, energy_diff, distance, mom, sm_on);
    ASSERT_GT(r_W, 0.0); // otherwise this scenario needs braking, not coasting -- pick different vi/vf/distance if this fails

    double expected_time = p::time_to_reach_velocity(vf, vi, r_W / 1000.0, sm_on);
    auto actual = p::time_to_reach_speed_over_distance(vi, vf, distance, mom, sm_on);

    ASSERT_TRUE(actual.has_value());
    EXPECT_NEAR(actual->time_s, expected_time, 1e-3);
}

// Accelerating branch, using two distances that both comfortably reach vf
// well before running out -- isolates just the "cruise the leftover
// distance" part of the formula via a *difference* between the two results,
// without needing an independent reference for the harder "how long does
// the tapering-acceleration phase itself take" part: since both distances
// share the same acceleration phase (same vi, vf, mom, sm_on), the only
// difference between the two results should be (distance2-distance1)/vf_ms.
TEST(TimeToReachSpeedOverDistanceProperties, AcceleratingCruiseTimeScalesWithLeftoverDistance) {
    double vi = 100.0, vf = 150.0;
    double distance1 = 1000.0, distance2 = 1500.0;
    bool mom = false, sm_on = false;

    auto result1 = p::time_to_reach_speed_over_distance(vi, vf, distance1, mom, sm_on);
    auto result2 = p::time_to_reach_speed_over_distance(vi, vf, distance2, mom, sm_on);
    ASSERT_TRUE(result1.has_value());
    ASSERT_TRUE(result2.has_value());

    double expected_extra_time = (distance2 - distance1) / (vf / 3.6);
    EXPECT_NEAR(result2->time_s - result1->time_s, expected_extra_time, 1e-2);
}

// Accelerating branch, distance far too short to reach vf at all -- even a
// single DELTA_T step covers a fraction of a meter at these speeds, so 1m
// is nowhere near enough to gain 50 km/h. Should hit the explicit
// "speed not reachable" guard and return nullopt, not fall through to
// compute a nonsense cruise time off a near-zero or negative leftover distance.
TEST(TimeToReachSpeedOverDistanceEdgeCase, ReturnsNulloptWhenDistanceTooShortToReachTarget) {
    auto result = p::time_to_reach_speed_over_distance(100.0, 150.0, 1.0, false, false);
    EXPECT_FALSE(result.has_value());
}