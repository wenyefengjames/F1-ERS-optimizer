#include <gtest/gtest.h>
#include "../include/battery.h"

TEST(BatteryTest, HarvestNeverExceedsCapacity) {
    Battery b(3.9, 0.0, false);
    b.harvest(1.0);
    EXPECT_LE(b.get_battery_charge(), 4.0);
}

// ---- Fixture: shared setup for tests that all start from the same baseline ----
// Avoids repeating "Battery b(...)" in every single TEST() below. SetUp() runs
// fresh before EACH test — you always start from a clean, known state, tests
// never leak state into each other.
class BatteryTestQualify : public ::testing::Test {
    protected:
        Battery battery = Battery(2.0, 0.0, false);
};

// ---- Normal case ----
TEST_F(BatteryTestQualify, HarvestBelowBothCapsAppliesInFull) {
    battery.harvest(1.0);
    EXPECT_DOUBLE_EQ(battery.get_battery_charge(), 3.0);
    EXPECT_DOUBLE_EQ(battery.get_harvest_charge(), 1.0);
}


// Check if the Constructor of Battery class assigns values correctly
TEST_F(BatteryTestQualify, TestCorrectAssignmentOfConstructor) {
    Battery low_charge_battery(0.5, 5.5, false);
    EXPECT_DOUBLE_EQ(low_charge_battery.get_harvest_charge(), 5.5);
    EXPECT_DOUBLE_EQ(low_charge_battery.get_battery_charge(), 0.5);
}

// ---- Boundary case #1: capped by battery CAPACITY specifically ----
// This is the case that's actually interesting for this codebase: harvest()
// has two independent caps (battery_capacity vs harvest_limit), and a bug
// this session specifically came from conflating which one was binding.
// Starting at 2.0/4.0 capacity, requesting 3.0 should cap at the 2.0
// headroom — and harvest_charge should only register that same capped 2.0,
// not the full 3.0 requested.
TEST_F(BatteryTestQualify, HarvestCappedByBatteryCapacity) {
    battery.harvest(3.0); // would overshoot 4.0 capacity if uncapped
    EXPECT_DOUBLE_EQ(battery.get_battery_charge(), 4.0);
    EXPECT_DOUBLE_EQ(battery.get_harvest_charge(), 2.0); // capped amount, not 3.0
}

// ---- Boundary case #2: capped by the per-lap HARVEST LIMIT specifically ----
// Deliberately keeps battery_charge low (plenty of capacity headroom) so
// the harvest_limit is the ONLY thing that can be binding here — isolates
// the two caps from each other rather than testing them tangled together.
TEST_F(BatteryTestQualify, HarvestCappedByHarvestLimit) {
    Battery low_charge_battery(0.5, 5.5, false); // 0.5/4.0 charge, 5.5/6.0 harvested already
    low_charge_battery.harvest(1.0);              // would overshoot the 6.0 harvest_limit
    EXPECT_DOUBLE_EQ(low_charge_battery.get_harvest_charge(), 6.0);
    EXPECT_DOUBLE_EQ(low_charge_battery.get_battery_charge(), 1.0); // only the capped 0.5 got added
}

// ---- Deploy's clamp-at-zero boundary ----
TEST_F(BatteryTestQualify, DeployNeverGoesNegative) {
    battery.deploy(5.0); // more than the 2.0 available
    EXPECT_DOUBLE_EQ(battery.get_battery_charge(), 0.0);
}

// ---- Simple boolean-returning queries: no fixture needed, standalone TEST() is fine ----
TEST(BatteryQueryTest, IsBatteryFullDetectsExactCapacity) {
    Battery full(4.0, 0.0, false);
    EXPECT_TRUE(full.is_battery_full());
}

class BatteryTestRace : public ::testing::Test {
    protected:
        Battery battery = Battery(2.0, 0.0, true); 
};

// Check if the harvest limit is correctly set to 8.5
TEST_F(BatteryTestRace, HarvestCap) {
    battery.set_harvest(9.0);
    EXPECT_DOUBLE_EQ(battery.get_harvest_charge(), 8.5);
    EXPECT_DOUBLE_EQ(battery.get_harvest_limit(), 8.5);
}

// Check if the battery charge is correctly capped by harvest limit
TEST_F(BatteryTestRace, BatteryChargeCappedByHarvestLimit){
    battery.set_harvest(8.0);
    battery.harvest(1.0); // Should only harvest 0.5 because of the cap

    EXPECT_DOUBLE_EQ(battery.get_harvest_charge(), 8.5);
    EXPECT_DOUBLE_EQ(battery.get_battery_charge(), 2.5);
}

// Check if the harvest charge is correctly capped by battery capacity
TEST_F(BatteryTestRace, HarvestChargeCappedByBatteryCapacity){
    battery.set_battery(3.5);
    battery.harvest(1.0); // Should only harvest 0.5 because of the battery capacity

    EXPECT_DOUBLE_EQ(battery.get_harvest_charge(), 0.5);
    EXPECT_DOUBLE_EQ(battery.get_battery_charge(), 4.0);
}

// ---- Parameterized test: same logic, many (input, expected) pairs ----
// Worth knowing about now because it's exactly what you'll want for
// physics.cpp once you have known (input -> expected output) pairs to check
// against — one test body, a table of cases, instead of copy-pasting
// near-identical TESTs by hand.
struct ChargeCase { double deploy, harvest, battery_charge, harvest_charge; bool expected; };

class CheckAllowChargeTest : public ::testing::TestWithParam<ChargeCase> {};

TEST_P(CheckAllowChargeTest, MatchesExpected) {
    ChargeCase c = GetParam();
    Battery b(c.battery_charge, c.harvest_charge, false);
    EXPECT_EQ(b.check_allow_charge(c.deploy, c.harvest), c.expected);
}

INSTANTIATE_TEST_SUITE_P(VariousCases, CheckAllowChargeTest, ::testing::Values(
    ChargeCase{0.0, 1.0, 2.0, 5.0, true},   // plenty of headroom both ways
    ChargeCase{0.0, 2.0, 2.0, 5.0, false},  // would exceed the 6.0 harvest limit
    ChargeCase{5.0, 0.0, 2.0, 0.0, false}   // can't deploy more than you have
));