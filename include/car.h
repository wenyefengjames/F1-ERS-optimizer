#pragma once
#include <string>
#include "battery.h"

class Car {
    private:
        const int horsepower;
        const int weight;
        const int energy_cap = 350;
        const int braking_force;
        bool race_mode;
        double speed;
        double delta = 0.0;
        Battery battery;

    public:
        Car(bool race_mode);

        // Harvesting methods
        double braking_harvest(double current_speed, double target_speed, double car_weight); 
        double coasting_harvest();
        double superclipping();
        double partial_throttle_harvest(double throttle_percentage);

        // Deployment
        double deployment();
        
        // The curve of how the deployment rate decreases
        double taper_curve();




};