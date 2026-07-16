#pragma once
#include <string>
#include "battery.h"

class Car {
    private:
        const double ICE = 400;            // measured in kW, approximatedly 536hp
        const double mass = 768;           //kg
        const double MGU_K = 350;          // measured in kW 
        const double braking_decel = 5.5*9.8;      // meansured in ms^-2
        bool race_mode;
        bool mom;
        double speed = 0.0;
        double delta = 0.0;
        Battery battery;

    public:
        Car(bool race_mode, bool mom);

        // Harvesting methods
        double braking_harvest(double target_speed); 
        double coasting_harvest(double time);
        double superclipping(double clip_rate, double time);
        double partial_throttle_harvest(double throttle_percentage, double time);

        // Deployment
        double deployment(double energy);
        
        // The curve of how the deployment rate decreases
        double taper_curve(double energy);

        double ke_to_speed(double energy);

        double drag();

        // Getters
        Battery get_battery();

        // Setters




};