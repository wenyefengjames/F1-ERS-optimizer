#include "battery.h"
#include <algorithm>

Battery::Battery(double battery_charge, double harvest_charge, bool race_mode){
    set_battery(battery_charge);
    set_harvest(harvest_charge);
    set_race_mode(race_mode);
}

// Harvesting -------------------------------------------------------------------------

// Calculate the amount of energy we can harvest through reaching target speed
void Battery::braking_harvest(double current_speed, double target_speed, double car_weight){}
void Battery::coasting_harvest(){}
void Battery::superclipping(){}
void Battery::partial_throttle_harvest(double throttle_percentage){}

// Harvest energy to battery
void Battery::harvest(double new_charge){
    double harvestable_amount;

    // Check if the battery or harvesting has reached the limit
    if(this->is_harvest_full() == false && this->is_battery_full() == false){
        harvestable_amount = std::min(this->harvest_limit - this->harvest_charge,
                                      this->battery_capacity - this->battery_charge); 

        // If new amount of harvest is less than the amount that we can recharge, recharge to the limit
        if(harvestable_amount <= new_charge){            
            set_battery(harvestable_amount);
            set_harvest(harvestable_amount);

        } 
        else{
            set_battery(this->battery_charge + new_charge);
            set_harvest(this->harvest_charge + new_charge);
        }
    }
}

// Return if the battery is full, to 4.0MJ
bool Battery::is_battery_full(){
    return this->battery_charge == this->battery_capacity;
}

// Return if the battery is empty
bool Battery::is_battery_empty(){
    return this->battery_charge == 0.0;
}

// Return if the maximum harvesting energy reached in this lap
bool Battery::is_harvest_full(){
    return this->harvest_charge == this->harvest_limit;
}

// Reset the total harvesting energy to 0 to prepare for next lap
void Battery::reset_harvest(){
    this->harvest_charge = 0.0;
}

// Deployment -------------------------------------------------------------------------

// Deployment the amount of charge given if there is enough charge within the battery
void Battery::deployment(double deploy_amount){
    this->battery_charge = std::max(this->battery_charge - deploy_amount, 0.0);
}

// Getters -------------------------------------------------
double Battery::get_battery_charge(){
    return this->battery_charge;
}

double Battery::get_harvest_charge(){
    return this->harvest_charge;
}

bool Battery::get_race_mode(){
    return this->race_mode;
}

// Setters -------------------------------------------------
void Battery::set_battery(double battery_charge){
    this->battery_charge = std::min(this->battery_charge + battery_charge, this->battery_capacity);
}

void Battery::set_harvest(double harvest_charge){
    this->harvest_charge = std::min(this->harvest_charge + harvest_charge, this->harvest_limit);
}


// Change race mode to new race mode
// Change maximum harvest energy in a lap based on the new race mode
void Battery::set_race_mode(bool new_mode){

    // 'true' represents race mode, 'false' represents qualifying mode
    if(new_mode == true){   
        this->harvest_limit = this->race_harvest_limit;
    } 
    else{   
        this->harvest_limit = this->qualifying_harvest_limit;
    }

    this->race_mode = new_mode;
}


