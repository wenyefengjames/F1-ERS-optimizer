#pragma once
#include "car.h"
#include "track.h"
#include "physics.h"
#include <optional>
#include <vector>

// This struct holds the value pair of battery cost with respect to delta time
struct Option {
    double battery;
    double delta;
};


class Optimizer{

    private:
        const int battery_buckets = 41;
        const int partition_size = 15;      // The length that distance of segment needs to be divided by
        const double bucket_size = 0.1;     // The size of energy bucket being used in each step, in MJ
        bool race_mode;
        bool mom;
        Track circuit = Track();
        Car car = Car(race_mode, mom);
        std::vector<double> table;    // Table for memoization
        std::vector<std::optional<Option>> choice;   // Table for path reconstruction

        // Helper function to flatten the 3D table into 1D by translating index positions
        int index_helper(int i, double b, double e);

        std::vector<Option> option_table_straight(int seg_index, double initial_battery);
        std::vector<Option> option_table_slowcorner(int seg_index);
        std::vector<Option> option_table_fastcorner(int seg_index, double initial_battery);  
        Option best_option_for_bucket(double energy_J, int length, double exit_speed, double target_speed,
                                      double initial_battery);

    public:
        Optimizer(bool race_mode, bool mom);

        double main_optimizing_loop();
        double dp_algorithm(int index, Battery battery, double ending_battery);
        double estimate_deploy_distance();
        std::vector<Option> path_reconstruction(double battery, double ending_battery);
        std::vector<Option> segment_options(int seg_index, double initial_battery);

};