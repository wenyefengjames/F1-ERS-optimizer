#include <gtest/gtest.h>
#include "../include/battery.h"

TEST(BatteryTest, HarvestNeverExceedsCapacity) {
    Battery b(3.9, 0.0, false);
    b.harvest(1.0);
    EXPECT_LE(b.get_battery_charge(), 4.0);
}
