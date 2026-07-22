#pragma once
#include <string>
#include "battery.h"

class Car {
    private:
        const double ICE = 400;            // measured in kW, approximatedly 536hp
        const double MASS_KG = 768;           //kg
        const double MGU_K = 350;          // measured in kW 
        const double braking_decel = 5.5*9.8;      // meansured in ms^-2
        bool race_mode;
        bool mom;
        double speed = 0.0;
        double delta = 0.0;
        Battery battery;

    public:
        Car(bool race_mode, bool mom);

        // Deployment
        double deployment(double energy);

        // Getters
        Battery get_battery();

        // Setters

};