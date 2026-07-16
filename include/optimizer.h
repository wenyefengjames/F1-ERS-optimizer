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
        bool race_mode;
        bool mom;
        Track circuit = Track();
        Car car = Car(race_mode, mom);
        std::vector<double> table;    // Table for memoization
        std::vector<std::optional<Option>> choice;   // Table for path reconstruction

        // Helper function to flatten the 3D table into 1D by translating index positions
        int index_helper(int i, double b, double e);

    public:
        Optimizer(bool race_mode, bool mom);

        double main_optimizing_loop();
        double dp_algorithm(int index, Battery battery, double ending_battery);
        double estimate_deploy_distance();
        std::vector<Option> path_reconstruction(double battery, double ending_battery);
        std::vector<Option> segment_options(int seg_index, double initial_battery);

        std::vector<Option> straight_to_fast(int seg_index, double initial_battery);
        std::vector<Option> straight_to_slow(int seg_index, double initial_battery);
        

};