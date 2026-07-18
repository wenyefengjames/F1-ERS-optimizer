#include "../include/optimizer.h"
#include <cmath>
#include <limits>

namespace p = physics;

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

double Optimizer::main_optimizing_loop(){
    return {};
}

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
    std::vector<Option> current_segment = segment_options(index, battery.get_battery_charge());

    bool is_last_segment = (index == circuit.size() - 1);

    // Main DP loop
    for (const Option& option : current_segment){
        total_time = std::numeric_limits<double>::infinity();
        remaining_time = 0.0;

        // Needs to check that the reminaing battery is at least 
        // the same amount of ending_battery at the end of the last segment
        bool ending_battery_ok = !is_last_segment ||
        (battery.get_battery_charge() + option.battery >= ending_battery);

        if (battery.check_allow_charge(option.battery) && ending_battery_ok){
            Battery next_battery = battery;
            if (option.battery >= 0) next_battery.harvest(option.battery);
            else next_battery.deploy(-option.battery);

            remaining_time = dp_algorithm(index + 1, next_battery, ending_battery);
            total_time = remaining_time + option.delta;
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
// initial_battery should be in MJ
std::vector<Option> Optimizer::segment_options(int seg_index, double initial_battery){
    std::vector<Option> option_table;    

    if (circuit.at(seg_index)->get_type() == "SlowCorner"){
        int length = circuit.at(seg_index)->get_length();
        int bucket_num = std::round(p::BATTERY_CAPACITY / bucket_size + 1); // 1 for neutral, rest goes to harvesting. Not allowed to deploy
        double entry_speed = 0.0;
        double min_speed = 0.0;

    } else if(circuit.at(seg_index)->get_type() == "FastCorner"){

    } else if(circuit.at(seg_index)->get_type() == "Straight"){

        return option_table_straight(seg_index, initial_battery);      
    }
    return option_table;
}

// Producing a vector of Options for a Segment of Straight following with a FastCorner
std::vector<Option> Optimizer::option_table_straight(int seg_index, double initial_battery){
    std::vector<Option> option_table;
    int length = circuit.at(seg_index)->get_length();
    int bucket_num = std::round(p::BATTERY_CAPACITY / bucket_size * 2 + 1); // 1 for neutral, half of the rest goes to harvesting, other goes to deploy 
    double target_speed = 0.0;
    double energy_bucket_J = 0.0;
    double exit_speed = 0.0;

    // Type checking and casting for the previous segment, and next segment ----------------------------

    // Extract the exit speed from the previous segment, doesnt matter what type of corner it is
    Segment* prev_segment = circuit.prev(seg_index);
    if(prev_segment->get_type() == "SlowCorner") {
        SlowCorner* corner = static_cast<SlowCorner*>(prev_segment);
        exit_speed = corner->get_exit_speed();
    }
    else if(prev_segment->get_type() == "FastCorner"){
        FastCorner* corner = static_cast<FastCorner*>(prev_segment);
        exit_speed = corner->get_exit_speed();
    }
    
    // Extract the target speed and length of the next segment, fast and slow corners are different here
    Segment* next_segment = circuit.next(seg_index);
    if(next_segment->get_type() == "SlowCorner") {
        SlowCorner* corner = static_cast<SlowCorner*>(next_segment);
        target_speed = corner->get_entry_speed();
    }
    else if(next_segment->get_type() == "FastCorner"){
        FastCorner* corner = static_cast<FastCorner*>(next_segment);
        target_speed = corner->get_apex_min_speed();

        // Assumption that the apex of the next corner is at exactly half way of the corner
        // Which is where the apex_min_speed lies
        length += corner->get_length() * 0.5;
    }
    // --------------------------------------------------------------------------------------------------

    // Option table generating loop
    for (int energy = 0; energy < bucket_num; energy++){
        energy_bucket_J = (energy * bucket_size - p::BATTERY_CAPACITY) * 1000000;

        // Find the optimal time for the given energy bucket
        Option temp = best_option_for_bucket(energy_bucket_J, length, exit_speed, target_speed, initial_battery);
        option_table.push_back(temp);
    }

    return option_table;
}

// For a given total energy and total length, return a Option struct containing estimated optimal length of deploying
Option Optimizer::best_option_for_bucket(double energy_J, int length, double exit_speed, 
                                         double target_speed, double initial_battery){
    double best_time = std::numeric_limits<double>::infinity();
    double total_time = std::numeric_limits<double>::infinity();
    const double ke_target_speed = p::kinetic_energy(target_speed); // loop invariant, so precompute
    const int full_engine_power = p::MGU_K + p::ICE; // loop invariant

    // Find the optimal time for the given energy bucket
    for (int dis = 0; dis < length / partition_size; dis++){
        total_time = std::numeric_limits<double>::infinity();

        const double deploy_dis = dis * partition_size;
        const double harvest_dis = length - deploy_dis;

        const double ke_gained = p::work_done_with_drag(full_engine_power, exit_speed, deploy_dis);
        const double speed = p::reverse_ke(exit_speed, ke_gained);
        const double time_deploying = p::time_to_reach_velocity(speed, exit_speed, full_engine_power);

        const double energy_deployed = time_deploying * p::MGU_K * 1000;

        // If the amount of energy deployed is more than what the battery have, break out of this loop
        // Because the deployment distance will only keep increasing, so the battery wont have enough for distance longer than current
        if(energy_deployed >= initial_battery * 1000000){
            break;
        }

        const double ke_diff = p::kinetic_energy(speed) - ke_target_speed;
        const double engine_power = p::required_power(speed, -ke_diff, harvest_dis);   // in Watts, not kW
                    
        // Stop the loop if it requires the engine power output to be less than 0 (meaning it requires breaking)
        if(engine_power < 0){
            break;
        }

        // Limit the range of recharge
        double recharge_power = p::ICE*1000 - engine_power;

        double time_harvesting = 0.0;
        
        // Recharge power has to be positive
        if (recharge_power <= 0){
            continue;
        }

        if (recharge_power >= p::MGU_K * 1000)   recharge_power = p::MGU_K * 1000;

        time_harvesting = (energy_J + p::MGU_K*1000 * time_deploying)/(recharge_power);

        const double energy_harvested = time_harvesting * recharge_power;
        total_time = time_harvesting + time_deploying;

        // Update best time
        if (total_time < best_time){
            best_time = total_time;
        }
    }

    Option output = {energy_J/1000000, best_time};
    return output;
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

double Optimizer::estimate_deploy_distance(){
    return {};
}
