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

class Optimizer{

    private:
        const int partition_size = 10;      // The length that distance of segment needs to be divided by
        const double bucket_size = 0.05;     // The size of energy bucket being used in each step, in MJ
        const int battery_buckets = 4.0/bucket_size + 1;
        bool race_mode;
        bool mom;
        Track circuit = Track();
        Car car = Car(race_mode, mom);
        const int harvest_buckets = car.get_battery().get_harvest_limit() / bucket_size + 1;
        std::vector<double> table;    // Table for memoization
        std::vector<std::optional<Option>> choice;   // Table for path reconstruction

        std::vector<Option> option_table_straight(int seg_index, double initial_battery);
        std::vector<Option> option_table_slowcorner(int seg_index);
        std::vector<Option> option_table_fastcorner(int seg_index, double initial_battery);  
        std::vector<Option> best_option_for_bucket(int length, double exit_speed, 
                                         double target_speed, double initial_battery);

    public:
        // Helper function to flatten the 4D table into 1D by translating index positions
        int index_helper(int i, double b, double e, double h);

        Optimizer(bool race_mode, bool mom);

        double main_optimizing_loop(int seg_index, double initial_battery, double ending_battery, double harvest);
        double dp_algorithm(int index, Battery battery, double ending_battery);
        double estimate_deploy_distance();
        std::vector<Option> path_reconstruction(int starting_index, double battery, double ending_battery, double harvest);
        std::vector<Option> segment_options(int seg_index, double initial_battery);
};