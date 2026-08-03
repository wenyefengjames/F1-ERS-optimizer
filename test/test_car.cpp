#include <gtest/gtest.h>
#include "../include/car.h"

class TestCar : public ::testing::Test {

    protected:
        Car car = Car(true /*race mode*/, true /*mom*/);
};

// Testing that the Constructor correctly constructs the car object
// race=true + mom=true means the harvest limit includes the +0.5MJ MOM
// bonus (8.5 base + 0.5 = 9.0), not just the plain race cap.
TEST_F(TestCar, CorrectConstructor){
    EXPECT_EQ(car.get_race_mode(), true);
    EXPECT_EQ(car.get_mom(), true);
    EXPECT_EQ(car.get_battery().get_battery_charge(), 4.0);
    EXPECT_EQ(car.get_battery().get_harvest_charge(), 0.0);
    EXPECT_EQ(car.get_battery().get_harvest_limit(), 9.0);
}

// Testing that changing race mode also changes harvest limit of the battery
// Testing mom setter as well
TEST_F(TestCar, ChangeRaceMode){
    car.set_race_mode(false);
    car.set_mom(false);
    EXPECT_EQ(car.get_race_mode(), false);
    EXPECT_EQ(car.get_mom(), false);
    EXPECT_EQ(car.get_battery().get_battery_charge(), 4.0);
    EXPECT_EQ(car.get_battery().get_harvest_charge(), 0.0);
    EXPECT_EQ(car.get_battery().get_harvest_limit(), 6.0);
}

// Regression check at the Car level: Car::set_mom used to forward to a
// Battery setter that stacked +0.5 on every call. The fixture already
// constructs with mom=true, so calling set_mom(true) again should stay at
// 9.0, not climb to 9.5.
TEST_F(TestCar, RepeatedSetMomDoesNotStackTheHarvestBonus){
    car.set_mom(true);
    car.set_mom(true);
    EXPECT_DOUBLE_EQ(car.get_battery().get_harvest_limit(), 9.0);
}