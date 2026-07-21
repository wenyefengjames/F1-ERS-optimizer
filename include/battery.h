#pragma once
#include <string>

class Battery {
    private:
        const double qualifying_harvest_limit = 6.0;  // Maximum energy allowed to harvest in a lap in qualifying, in MJ
        const double race_harvest_limit = 8.5;        // Maximum energy allowed to harvest in a lap in race, in MJ
        const double battery_capacity = 4.0;          // Maximum capacity of battery, in MJ
        double battery_charge = 0.0;      // Can only be between 0-4.0, unit of MJ
        double harvest_charge = 0.0;      // Total harvested energy in a lap
        double harvest_limit;       // Maximum energy allowed to harvest in a lap, depending on race mode
        bool race_mode;             // True being race mode, False being qualifying mode

    public:
        Battery(double battery_charge, double total_harvest, bool race_mode);

        // Harvest
        void harvest(double new_charge);

        // Common utility methods
        bool is_battery_full();
        bool is_battery_empty();
        bool is_harvest_full();
        void reset_harvest();
        
        bool check_allow_charge(double deploy, double harvest);
        double avaliable_charge();

        // Deployment
        void deploy(double deploy_amount);

        // Getters --------------------------------------
        double get_battery_charge();
        double get_harvest_charge();
        double get_harvest_limit();
        bool get_race_mode();

        // Setters --------------------------------------
        void set_race_mode(bool new_race_mode);
        void set_battery(double battery_charge);
        void set_harvest(double set_harvest);


};