#pragma once
#include <string>
#include "battery.h"

class Car {
    private:
        const int ICE = 400;        // measured in kW, approximatedly 536hp
        const int mass = 768;       //kg
        const int MGU_K = 350;      // measured in kW
        const int braking_force = 0;
        bool race_mode;
        bool mom;
        double speed = 0.0;
        double delta = 0.0;
        Battery battery;

    public:
        Car(bool race_mode, bool mom);

        // Harvesting methods
        double braking_harvest(double target_speed); 
        double coasting_harvest();
        double superclipping();
        double partial_throttle_harvest(double throttle_percentage);

        // Deployment
        double deployment(double energy);
        
        // The curve of how the deployment rate decreases
        double taper_curve(double energy);

        double ke_to_speed(double energy);

        double drag();




};