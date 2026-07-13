#include "../include/car.h"
#include <cmath>

Car::Car(bool race_mode, bool mom) 
        : race_mode(race_mode), mom(mom), battery(4.0, 0.0, race_mode){}

// Harvesting methods
double Car::braking_harvest(double target_speed){
    //TO DO
    return 0.0;
}
double Car::coasting_harvest(){
    //TO DO
    return 0.0;
}
double Car::superclipping(){
    //TO DO
    return 0.0;
}
double Car::partial_throttle_harvest(double throttle_percentage){
    //TO DO
    return 0.0;
}

// Deployment. Input: the amount of time that the MGU-K deploys
// Output: the change in car speed and its delta compared to the theoretical time.
double Car::deployment(double time){
    double r = this->MGU_K;    // The rate of deployment, measured in kW
    double energy = time * r * 1000;// Total energy deployed, measured in J
    double d;                       // Distance travelled during deployment
    double v = this->speed / 3.6;   // Current speed in m/s
    int m = this->mass;

    this->battery.deploy(energy/1000000);
    this->speed = ke_to_speed(energy);

    d = (m/3.0*r) * (std::pow((v*v + 2.0 * r * time / m), 3.0/2.0) - pow(v,3));
    r = this->ICE * 1000;

    // time taken if no deployment to travel the same distance
    double s = (m/2.0*r) * (pow(3.0*r*d/m + pow(v,3), 2.0/3.0) - v*v); 

    this->delta += s - time; // Change in time

    return 0.0;
}
        
// The curve of how the deployment rate decreases
// Assuming the power output decreases linearly with respect to speed
double Car::taper_curve(double energy){
    double power_output; // in kW

    if(mom){
        if(this->speed <= 337){
            power_output = 350;
        } 
        else{
            power_output = 350 + (this->speed - 337)*-19.44;
        }
    }
    else{
        if(this->speed <= 290){
            power_output = 350;
        } 
        else{
            power_output = 350 + (this->speed - 290)*-5.38;
        }
    }

    return power_output;
}

// Calculate new speed based on the current speed and extra energy
// energy need to be in unit of J, not kJ or MJ
double Car::ke_to_speed(double energy){
    double speed_ms = this->speed / 3.6; // Convert km/h into m/s

    double new_speed = std::sqrt(speed_ms * speed_ms + 2 * energy / this->mass);

    return new_speed * 3.6; // Convert m/s into km/h
} 