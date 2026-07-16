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
    std::vector<Option> current_segment = segment_options(index, battery.get_battery_charge());

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
// initial_battery should be in MJ
std::vector<Option> Optimizer::segment_options(int seg_index, double initial_battery){

    std::vector<Option> option_table;

    if (circuit.at(seg_index)->get_type() == "SlowCorner"){
        int length = circuit.at(seg_index)->get_length();
        double bucket_size = 0.1;               // In Mj
        int bucket_num = p::BATTERY_CAPACITY / bucket_size + 1; // 1 for neutral, rest goes to harvesting. Not allowed to deploy
        double entry_speed = 0.0;
        double min_speed = 0.0;

    } else if(circuit.at(seg_index)->get_type() == "FastCorner"){

    } else if(circuit.at(seg_index)->get_type() == "Straight"){

        // Treating fast corners and slow corners differently. 
        if(circuit.next(seg_index)->get_type() == "FastCorner"){
            return straight_to_fast(seg_index, initial_battery);
        }
        else{
        
        }        

    }
    return option_table;
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

// Producing a vector of Options for a Segment of Straight following with a FastCorner
std::vector<Option> Optimizer::straight_to_fast(int seg_index, double initial_battery){
    std::vector<Option> option_table;
    int length = circuit.at(seg_index)->get_length();
    int partition_size = 15;
    double bucket_size = 0.1;         // In Mj
    int bucket_num = std::round(p::BATTERY_CAPACITY / bucket_size * 2 + 1); // 1 for neutral, half of the rest goes to harvesting, other goes to deploy 
    double exit_speed = 0.0;
    double target_speed = 0.0;
    double best_time = std::numeric_limits<double>::infinity();
    double total_time = std::numeric_limits<double>::infinity();

    // Type checking and casting
    Segment* prev_segment = circuit.prev(seg_index);
    if(prev_segment->get_type() == "SlowCorner") {
        SlowCorner* corner = static_cast<SlowCorner*>(prev_segment);
        exit_speed = corner->get_exit_speed();
    }
    else if(prev_segment->get_type() == "FastCorner"){
        FastCorner* corner = static_cast<FastCorner*>(prev_segment);
        exit_speed = corner->get_exit_speed();
    }

    // Next segment doesnt need to go through checks, because it is definitely a fast corner
    FastCorner* corner = static_cast<FastCorner*>(circuit.next(seg_index));
    target_speed = corner->get_apex_min_speed();

    for (int energy = 0; energy < bucket_num; energy++){
        double energy_bucket_J = (energy * bucket_size - p::BATTERY_CAPACITY) * 1000000;
        double energy_bucket_MJ = energy * bucket_size - p::BATTERY_CAPACITY;
        best_time = std::numeric_limits<double>::infinity();
        total_time = std::numeric_limits<double>::infinity();

        // Find the optimal time for the given energy bucket
        for (int dis = 0; dis < length / partition_size; dis++){
            int deploy_dis = dis * partition_size;
            int harvest_dis = length - deploy_dis;

            double ke_gained = p::work_done_with_drag(p::MGU_K + p::ICE, exit_speed, deploy_dis);
            double speed = p::reverse_ke(exit_speed, ke_gained);
            double time_deploying = p::time_to_reach_velocity(speed, exit_speed, p::MGU_K + p::ICE);

            double energy_deployed = time_deploying * p::MGU_K * 1000;

            // If the amount of energy deployed is more than what the battery have, break out of this loop
            // Because the deployment distance will only keep increasing, so the battery wont have enough for distance longer than current
            if(energy_deployed >= initial_battery * 1000000){
                break;
            }

            double ke_diff = p::kinetic_energy(speed) - p::kinetic_energy(target_speed);
            double engine_power = p::required_power(speed, -ke_diff, harvest_dis);   // in Watts, not kW
                    
            // Stop the loop if it requires the engine power output to be less than 0 (meaning it requires breaking)
            if(engine_power < 0){
                break;
            }

            // Limit the range of recharge
            double recharge_power = p::ICE*1000 - engine_power;
            if (recharge_power >= p::MGU_K * 1000){
                recharge_power = p::MGU_K * 1000;
            }
            else if (recharge_power <= 0){
                recharge_power = 0;
            }

            double time_harvesting = (energy_bucket_J + p::MGU_K*1000 * time_deploying)/(recharge_power);
                    
            // Time has to be positive
            if (time_harvesting <= 0){
                continue;
            }

            double energy_harvested = time_harvesting * recharge_power;
            total_time = time_harvesting + time_deploying;

            // Update best time
            if (total_time < best_time){
                best_time = total_time;
            }
        }

        Option temp = {energy_bucket_MJ, best_time};
        option_table.push_back(temp);
    }

    return option_table;
}


double Optimizer::estimate_deploy_distance(){}