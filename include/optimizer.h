#pragma once
#include "car.h"
#include "track.h"
#include "physics.h"
#include <optional>
#include <vector>

// This struct holds the value pair of battery cost with respect to delta time
struct Option {
    double deploy;
    double harvest;
    double delta;
};

// This struct holds the information of how the deployment and harvesting is actually executed
struct ExecutionDetails{
    double deployment_distance;
    double deployment_rate;
    double harvest_distance;
    double harvest_rate;
    double braking_distance;
};

class Optimizer{

    private:
        const int partition_size = 10;          // The length that distance of segment needs to be divided by
        const double bucket_size = 0.05;        // The size of energy bucket being used in each step, in MJ
        const int battery_buckets = 4.0/bucket_size + 1;    // Number of buckets that the battery is divided into
        const double deployment_step_size = 25; // in kW, how much increase for each step of deployment
        bool race_mode;
        bool mom;
        // Track circuit = Track(); // TO DO REMOVE THIS COMMENT AFTER TESTING
        Car car = Car(race_mode, mom);
        const int harvest_buckets = car.get_battery().get_harvest_limit() / bucket_size + 1;
        std::vector<double> table;                      // Table for memoization
        std::vector<std::optional<Option>> choice;      // Table for path reconstruction
        std::vector<std::vector<Option>> option_table_lookup_table;     // A lookup table for every segment's option table
        std::vector<std::vector<ExecutionDetails>> execution_lookup_table;        // A lookup table for every Option's execution in each segment 

        // TO DO REMOVE THIS COMMENT AFTER TESTING
        // std::vector<Option> option_table_straight(int seg_index, double initial_battery);
        // std::vector<Option> option_table_slowcorner(int seg_index);
        // std::vector<Option> option_table_fastcorner(int seg_index, double initial_battery);  
        // std::vector<Option> best_option_for_bucket(double length, int seg_index, double exit_speed, 
        //                                             double target_speed, double initial_battery);
        // void initialize_option_table_lookup_table();

    public:
        Track circuit = Track();
        std::vector<Option> option_table_straight(int seg_index, double initial_battery);
        std::vector<Option> option_table_straight_new(int seg_index);
        std::vector<Option> option_table_slowcorner(int seg_index);
        std::vector<Option> option_table_fastcorner(int seg_index, double initial_battery);  
        std::vector<Option> option_table_corner(int seg_index); 
        std::vector<Option> best_option_for_bucket(double length, int seg_index, double exit_speed, 
                                                    double target_speed, double initial_battery);
        std::vector<Option> best_option_for_bucket_new(double length, int seg_index, double exit_speed, 
                                                    double target_speed, double initial_battery);
        void initialize_option_table_lookup_table();

        // Helper function to flatten the 4D table into 1D by translating index positions
        unsigned int index_helper(int i, double b, double e, double h);

        Optimizer(bool race_mode, bool mom);

        double main_optimizing_loop(int seg_index, double initial_battery, double ending_battery, double harvest);
        double dp_algorithm(int index, Battery battery, double ending_battery);
        std::vector<Option> path_reconstruction(int starting_index, double battery, double ending_battery, double harvest);
        std::vector<Option> segment_options(int seg_index, double initial_battery);
};