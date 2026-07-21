#include "../include/car.h"
#include "../include/physics.h"
#include <cmath>
#include <algorithm>

namespace p = physics;

Car::Car(bool race_mode, bool mom) 
        : race_mode(race_mode), mom(mom), battery(4.0, 0.0, race_mode){}

// Deployment. Input: the amount of time that the MGU-K deploys
// Output: the change in car speed and its delta compared to the theoretical time.
double Car::deployment(double time){
    double r = this->MGU_K;         // The rate of deployment, measured in kW
    double energy = time * r * 1000;// Total energy deployed, measured in J
    double d;                       // Distance travelled during deployment
    double v = this->speed / 3.6;   // Current speed in m/s
    double m = this->MASS_KG;

    this->battery.deploy(energy/1000000);
    this->speed = p::reverse_ke(this->speed, energy);

    d = (m/3.0*r) * (std::pow((v*v + 2.0 * r * time / m), 3.0/2.0) - pow(v,3));
    r = this->ICE * 1000;

    // time taken if no deployment to travel the same distance
    double s = (m/2.0*r) * (pow(3.0*r*d/m + pow(v,3), 2.0/3.0) - v*v); 

    this->delta += s - time; // Change in time

    return 0.0;
}

Battery Car::get_battery(){
    return battery;
}