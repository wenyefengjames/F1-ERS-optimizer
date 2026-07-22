#include <gtest/gtest.h>
#include "../include/car.h"

class TestCar : public ::testing::Test {

    protected:
        Car car = Car(true /*race mode*/, true /*mom*/);
};

// Testing that the Constructor correctly constructs the car object
TEST_F(TestCar, CorrectConstructor){
    EXPECT_EQ(car.get_race_mode(), true);
    EXPECT_EQ(car.get_mom(), true);
    EXPECT_EQ(car.get_battery().get_battery_charge(), 4.0);
    EXPECT_EQ(car.get_battery().get_harvest_charge(), 0.0);
    EXPECT_EQ(car.get_battery().get_harvest_limit(), 8.5);
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