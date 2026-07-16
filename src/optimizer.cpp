#include "../include/optimizer.h"
#include <cmath>
#include <limits>

Optimizer::Optimizer(bool race_mode, bool mom) : race_mode(race_mode), mom(mom){
    table.resize(circuit.size() * battery_buckets * battery_buckets, -1.0);
    choice.resize(circuit.size() * battery_buckets * battery_buckets, std::nullopt);
}

// i for index: which segment are we currently in
// b for battery level: since it is a stepping of 0.1 between 0-4. We will multiply this by 10 to give int
// e for ending battery level: same as b
int Optimizer::index_helper(int i, double b, double e){
    int b_bucket = static_cast<int>(std::round(b * 10));
    int e_bucket = static_cast<int>(std::round(e * 10));

    int value = e_bucket * circuit.size() * battery_buckets + (i * battery_buckets + b_bucket);
    return value;
}

double Optimizer::main_optimizing_loop(){}

// Index should be the index of the current segment of the circuit that we are on.
// battery should be passed in value, they are changed within the function without affecting the actual battery level,
// so that simulation can happen correctly
// ending_battery is the target battery level that we need to reach at the end of the last segment
double Optimizer::dp_algorithm(int index, Battery battery, double ending_battery){
    
    // Base case check
    if (index == circuit.size()) {
        return 0;
    }

    int i = index_helper(index, battery.get_battery_charge(), ending_battery);

    // Return value immediately if there is a memoization of the current state
    if (table.at(i) != -1){
        return table.at(i);
    }

    std::optional<Option> best_path;
    double best_time = std::numeric_limits<double>::infinity();
    double total_time = std::numeric_limits<double>::infinity();
    double remaining_time = 0.0;
    
    // This should be a vector of Option struct that gives all the strategy options for
    // the current segment of the race track that we are on. 
    std::vector<Option> current_segment = segment_options();

    // Main DP loop
    for (const Option& option : current_segment){
        total_time = std::numeric_limits<double>::infinity();
        remaining_time = 0.0;
        if (index != circuit.size() - 1){

            if (battery.check_allow_charge(option.battery)){
                Battery next_battery = battery;
                if (option.battery >= 0) next_battery.harvest(option.battery);
                else next_battery.deploy(-option.battery);

                remaining_time = dp_algorithm(index + 1, next_battery, ending_battery);
                total_time = remaining_time + option.delta;
            }
        }
        else{ // Needs to check that the reminaing battery is at least the same amount of ending_battery
              // at the end of the last segment
            if (battery.check_allow_charge(option.battery) && 
                battery.get_battery_charge() + option.battery >= ending_battery){

                Battery next_battery = battery;
                if (option.battery >= 0) next_battery.harvest(option.battery);
                else next_battery.deploy(-option.battery);

                remaining_time = dp_algorithm(index + 1, next_battery, ending_battery);
                total_time = remaining_time + option.delta;
            }
        }

        if (total_time < best_time){
            best_time = total_time;
            best_path = option;
        }
    }

    table.at(i) = best_time;
    choice.at(i) = best_path;

    return best_time;
}

// This function will create an array of strategy options for the current segment that we are on
std::vector<Option> Optimizer::segment_options(){
    return {};
}

// Reconstruct the path
// Limitation, only runs when battery is in range : 0 <= battery <= 4
// only runs after optimization is done, ie run on the same starting battery level as the table
std::vector<Option> Optimizer::path_reconstruction(double battery, double ending_battery){
    std::vector<Option> path;
    int index = 0;

    for (int i = 0; i < circuit.size(); i++){
        index = index_helper(i, battery, ending_battery);
        std::optional<Option> temp = choice.at(index);

        if(temp.has_value()){
            path.push_back(temp.value());
            battery += temp.value().battery;
        }
        else{ // Output an empty vector if there is no valid paths
            path.clear();
            return path;
        }
    }

    return path;
}


double Optimizer::estimate_deploy_distance(){}