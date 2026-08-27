#include "../include/optimizer.h"
#include <cmath>
#include <limits>
#include <algorithm>
#include <iostream>

namespace p = physics;

Optimizer::Optimizer(bool race_mode, bool mom) :
                     race_mode(race_mode), mom(race_mode ? mom : true){

    auto size = circuit.size() * static_cast<long long>(battery_buckets) * 
                static_cast<long long>(battery_buckets) * static_cast<long long>(harvest_buckets);

    table.resize(size, -1.0);
    choice.resize(size, std::nullopt);
    execution_lookup_table.resize(circuit.size());

    initialize_option_table_lookup_table();
}

// i for index: which segment are we currently in
// b for battery level: since it is a stepping of 0.1 between 0-4. We will multiply this by 10 to give int
// e for ending battery level: same as b
// h for harvest: to stop the algorithm from reaching over the harvest limit
unsigned int Optimizer::index_helper(int i, double b, double e, double h){
    int b_bucket = static_cast<int>(std::round(b * (1/bucket_size)));
    int e_bucket = static_cast<int>(std::round(e * (1/bucket_size)));
    int h_bucket = static_cast<int>(std::round(h * (1/bucket_size)));

    unsigned int value = h_bucket * (circuit.size() * battery_buckets * battery_buckets) + 
                (e_bucket * circuit.size() * battery_buckets + (i * battery_buckets + b_bucket));
    return value;
}

// Input: seg_index, the index of the segment of the track that you want to start the simulation on
//        initial_battery, the starting battery that you want to give the car, in MJ
//        ending_battery, how much leftover battery charge you need, in MJ
//        harvest, the starting harvest that you want to give the car, should be 0 by default
double Optimizer::main_optimizing_loop(int seg_index, double initial_battery, double ending_battery, double harvest){
    Battery battery = Battery(initial_battery, harvest, race_mode, mom);
    double best_laptime = dp_algorithm(seg_index, battery, ending_battery);
    std::vector<Option> deployment_choice = path_reconstruction(seg_index, initial_battery, ending_battery, harvest);

    double total_recharge = 0.0;
    double total_deploy = 0.0;

    double sector_1 = 0.0;
    double sector_2 = 0.0;
    double sector_3 = 0.0;

    std::cout << "Starting battery charge: " << initial_battery << "MJ \n";

    // Display all the choices made to give the final output
    for(const Option& op : deployment_choice){
        // std::cout << "--------------------------\n";
        std::cout << circuit.at(seg_index)->get_name() << '\t';
        std::cout << "Time spent: " << op.delta << '\t';
        std::cout << "Deployed: " << op.deploy << "MJ \t";
        std::cout << "Harvested: " << op.harvest << "MJ \n";
        // std::cout << "battery before harvest: " << battery.get_harvest_charge() << '\t';
        // std::cout << "uncapped harvest: " << battery.get_harvest_charge() + op.harvest << '\t';
        // std::cout << "Is harvest full?: " << (battery.get_harvest_charge() + op.harvest 
        //                                     <= battery.get_harvest_limit()) << '\n';


        // Calculate sector times
        if(seg_index >=0 && seg_index <= 6){
            sector_1 += op.delta;
        }
        else if (seg_index >=7 && seg_index <= 15){
            sector_2 += op.delta;
        }
        else{
            sector_3 += op.delta;
        }

        total_deploy += op.deploy;
        total_recharge += op.harvest;
        battery.deploy(op.deploy);
        battery.harvest(op.harvest);
        
        // std::cout << "battery after harvest: " << battery.get_harvest_charge() << '\t';
        // std::cout << "harvest_limit: " << battery.get_harvest_limit() << '\n';
        seg_index += 1;
    }
    std::cout << "--------------------------\n";
    std::cout << "Total amount of energy deployed: " << total_deploy << "MJ \t";
    std::cout << "Total amount of energy harvested: " << total_recharge << "MJ \n";
    std::cout << "Net change of battery: " << total_recharge - total_deploy << '\n';
    std::cout << "--------------------------\n";
    std::cout << "Sector times:\n";
    std::cout << "Sector 1: " << sector_1 << '\t';
    std::cout << "Sector 2: " << sector_2 << '\t';
    std::cout << "Sector 3: " << sector_3 << '\n';

    return best_laptime;
}

// Index should be the index of the current segment of the circuit that we are on.
// battery should be passed in value, they are changed within the function without affecting the actual battery level,
// so that simulation can happen correctly
// ending_battery is the target battery level that we need to reach at the end of the last segment
double Optimizer::dp_algorithm(int index, Battery battery, double ending_battery){
    
    // Base case check
    if (index == circuit.size()) {
        // TESTING: SHOULD BE REMOVED AFTER TESTING IS COMPLETE
        // std::cout << "ran out of track: " << '\n';
        return 0;
    }
    // TESTING: SHOULD BE REMOVED AFTER TESTING IS COMPLETE
    // std::cout << "current segment name: " <<  circuit.at(index)->get_name() << '\n';
    // std::cout << "prev segment name: " <<  circuit.prev(index)->get_name() << '\n';
    // std::cout << "next segment name: " <<  circuit.next(index)->get_name() << '\n';

    unsigned int i = index_helper(index, battery.get_battery_charge(), ending_battery, battery.get_harvest_charge());

    // Return value immediately if there is a memoization of the current state
    if (table.at(i) != -1){
        // TESTING: SHOULD BE REMOVED AFTER TESTING IS COMPLETE
        // std::cout << "apprently I found something already: " << table.at(i) << '\n';
        return table.at(i);
    }

    std::optional<Option> best_path;
    double best_time = std::numeric_limits<double>::infinity();
    double total_time = std::numeric_limits<double>::infinity();
    
    // This should be a vector of Option struct that gives all the strategy options for
    // the current segment of the race track that we are on. 
    // std::vector<Option> current_segment = segment_options(index, battery.get_battery_charge());
    const std::vector<Option>& current_segment = option_table_lookup_table[index];

    // TESTING: SHOULD BE REMOVED AFTER TESTING IS COMPLETE
    // std::cout << "size of the segment" << current_segment.size() << '\n';
    // for(const Option& op : current_segment){
    //     std::cout << "Deployment choice: " << op.deploy << '\t';
    //     std::cout << "Harvesting choice: " << op.harvest << '\t';
    //     std::cout << "Delta change: " << op.delta << '\n';
    // }

    bool is_last_segment = index == (circuit.size() - 1);

    // TESTING: SHOULD BE REMOVED AFTER TESTING IS COMPLETE
    // std::cout << "is_last_segment " << is_last_segment << '\n';

    // Main DP loop
    for (const Option& option : current_segment){
        total_time = std::numeric_limits<double>::infinity();

        // Needs to check that the reminaing battery is at least 
        // the same amount of ending_battery at the end of the last segment
        bool ending_battery_ok = !is_last_segment ||
        (battery.get_battery_charge() - option.deploy + option.harvest >= ending_battery);

        // TESTING: SHOULD BE REMOVED AFTER TESTING IS COMPLETE
        // std::cout << "ending_battery_ok " << ending_battery_ok << '\n';
        // std::cout << "Deploy: " << option.deploy << '\t';
        // std::cout << "Harvest: " << option.harvest << '\n';
        // std::cout << "battery charge: " << battery.get_battery_charge() << '\t';
        // std::cout << "harvest charge: " << battery.get_harvest_charge() << '\n';
        // std::cout << "ending battery check: " << ending_battery_ok << '\t';
        // std::cout << "check allow charge: " << battery.check_allow_charge(option.deploy, option.harvest) << '\t';
        // std::cout << "Check if-statement: " << (battery.check_allow_charge(option.deploy, option.harvest) && ending_battery_ok) << '\n';

        if (battery.check_allow_charge(option.deploy, option.harvest) && ending_battery_ok){

            Battery next_battery = battery;
            next_battery.deploy(option.deploy);
            next_battery.harvest(option.harvest);

            // std::cout << "current segment name: " <<  circuit.at(index)->get_name() << '\n';
            // std::cout << "next segment name: " <<  circuit.next(index)->get_name() << '\n';

            double remaining_time = dp_algorithm(index + 1, next_battery, ending_battery);
            total_time = remaining_time + option.delta;

            // std::cout << "remaining_time" << remaining_time << "\n";
            // std::cout << "total_time" << total_time << "\n";
        }
        // else{
        //     std::cout << "no update" << "\n";
        // }

        if (total_time < best_time){
            best_time = total_time;
            best_path = option;
        }
    }
    table.at(i) = best_time;
    choice.at(i) = best_path;

    return best_time;
}

// Reconstruct the path
// Limitation, only runs when battery is in range : 0 <= battery <= 4
// only runs after optimization is done, ie run on the same starting battery level as the table
std::vector<Option> Optimizer::path_reconstruction(int starting_index, double battery, double ending_battery, double harvest){
    std::vector<Option> path;
    unsigned int index = 0;

    for (int i = starting_index; i < circuit.size(); i++){
        index = index_helper(i, battery, ending_battery, harvest);
        std::optional<Option> temp = choice.at(index);

        if(temp.has_value()){
            path.push_back(temp.value());
            battery += temp.value().harvest - temp.value().deploy;
            harvest += temp.value().harvest;
        }
        else{ // Output an empty vector if there is no valid paths
            path.clear();
            return path;
        }
    }
    return path;
}


// This function will create an array of strategy options for the current segment that we are on
// initial_battery should be in MJ
std::vector<Option> Optimizer::segment_options(int seg_index){
    std::vector<Option> option_table;    

    if(circuit.at(seg_index)->get_type() == SegmentType::Straight){
        // std::cout << "Going into Straight" << "\n";
        return option_table_straight(seg_index);      
    }
    else{
        return option_table_corner(seg_index);
    }
    return option_table;
}

// TO DO
std::vector<Option> Optimizer::option_table_corner(int seg_index){

}

// REMOVE END =============================================================================================================================

// Producing a vector of Options for a Segment of Straight following with a FastCorner
std::vector<Option> Optimizer::option_table_straight(int seg_index){
    std::vector<Option> option_table;
    std::vector<ExecutionDetails> execution_table;
    double length = circuit.at(seg_index)->get_length();

    auto prev_corner = static_cast<Corner*>(circuit.prev(seg_index));
    auto next_corner = static_cast<Corner*>(circuit.next(seg_index));

    double starting_speed = prev_corner->get_exit_speed();
    double ending_speed = next_corner->get_entry_speed();
 
    bool sm = false;
    auto seg = static_cast<Straight*>(circuit.at(seg_index));
    std::vector<Option> output;

    // Build the braking lookup table ========================================================
    const double max_speed = 360; 
    const double v_step_size = 1;
    std::vector<TaperedDeploymentResult> braking_lookup_table;
    TaperedDeploymentResult init = {.speed_kmh = ending_speed, .energy_J = 0,
                                    .time_s = 0, .distance_m = 0};
    braking_lookup_table.push_back(init);
    
    // Iteratively build up a table of values based on previous values
    for(int v = 1; v < std::ceil((max_speed - ending_speed) / v_step_size); v++){
        const double prev_v_ms = braking_lookup_table[v - 1].speed_kmh / 3.6;
        const double braking_v_ms = (v * v_step_size + ending_speed) / 3.6;
        const double braking_decel = p::max_deceleration(braking_v_ms * 3.6, 0.0, 10.0, false);
        const double braking_dis = (braking_v_ms*braking_v_ms - prev_v_ms*prev_v_ms) / (2 * braking_decel) + braking_lookup_table[v - 1].distance_m;
        const double braking_time = (braking_v_ms - prev_v_ms) / braking_decel + braking_lookup_table[v - 1].time_s;
        const double braking_energy = braking_time * p::MGU_K * 1000;

        const TaperedDeploymentResult output = {.speed_kmh = braking_v_ms * 3.6, .energy_J = braking_energy,
                                                .time_s = braking_time, .distance_m = braking_dis};

        braking_lookup_table.push_back(output);
    }

    // TESTING

    // for(size_t i = 1; i < braking_lookup_table.size(); i++){
    //     std::cout << "Initial braking velocity: " << braking_lookup_table[i].speed_kmh << '\t';
    //     std::cout << "Braking time: " << braking_lookup_table[i].time_s << '\t';
    //     std::cout << "Braking distance: " << braking_lookup_table[i].distance_m << '\t';
    //     std::cout << "Harvested energy: " << braking_lookup_table[i].energy_J << '\t';
    //     std::cout << "Deceleration: " << (1 / 3.6) / (braking_lookup_table[i].time_s - braking_lookup_table[i-1].time_s) << '\n';
    // }

    // Find the optimal time for the given energy bucket
    for (int dis = 0; dis < circuit.at(seg_index)->get_length() / partition_size; dis++){
        for(int i = 0; i < p::MGU_K / deployment_step_size + 1; i++){
            // Deployment phase =============================================================
            const double deploy_rate = i * deployment_step_size;
            const double deploy_dis = dis * partition_size;

            TaperedDeploymentResult results  = p::energy_deployed_with_taper(starting_speed, deploy_dis, deploy_rate, seg->get_sm_start(), seg->get_sm_end(), mom);

            // const double speed = results.speed_kmh;
            // const double time_deploying = results.time_s;
            // double energy_deployed = results.energy_J;
            const double speed = results.speed_kmh;
            const double time_deploying = results.time_s;
            double energy_deployed = time_deploying * deploy_rate * 1000;
            
            // TESTING
            // std::cout << "energy_deployed: " << energy_deployed << "\t";
            // std::cout << "speed: " << speed << "\t";
            // std::cout << "time_deploying: " << time_deploying << "\n";

            // If the amount of energy deployed is more than what the battery have, break out of this loop
            // Because the deployment distance will only keep increasing, so the battery wont have enough for distance longer than current
            if(energy_deployed >= p::BATTERY_CAPACITY * 1000000) break;

            if(deploy_dis >= seg->get_sm_end()) sm = false;
            else sm = true;

            // Braking phase =============================================================
            for(size_t i = 0; i < braking_lookup_table.size(); i++){
                const double entry_speed = braking_lookup_table[i].speed_kmh;
                const double braking_dis = braking_lookup_table[i].distance_m;
                const double braking_energy = braking_lookup_table[i].energy_J;
                const double braking_time = braking_lookup_table[i].time_s;

                const double harvest_dis = length - results.distance_m - braking_dis;

                // Deployment distance and braking distance can't be more than the length of the segment
                if(braking_dis + deploy_dis > length) break;

                // The top speed is less than the starting speed for braking
                if(speed < entry_speed) break;

                // Harvest phase =============================================================
                auto result_for_time_energy = p::time_to_reach_speed_over_distance(speed, entry_speed, harvest_dis, mom, sm);

                if(!result_for_time_energy.has_value()) continue;

                const double energy_harvested = result_for_time_energy.value().energy_J;
                const double time_harvesting = result_for_time_energy.value().time_s;
                const double harvest_rate = energy_harvested / time_harvesting;

                // Harvesting rate exceeding the cap
                if(harvest_rate > p::MGU_K * 1000 or harvest_rate < 0) continue;

                const double total_time = time_harvesting + time_deploying;

                Option temp = {.deploy = results.energy_J, .harvest = (energy_harvested + braking_energy), .delta = total_time};
                ExecutionDetails exe = {.deployment_distance = deploy_dis, .deployment_rate = deploy_rate, .harvest_distance = harvest_dis,
                                        .harvest_rate = harvest_rate, .braking_distance = braking_dis};
                output.push_back(temp);
                execution_table.push_back(exe);

                // energy_deployed = bucket_size * std::ceil(energy_deployed / 1000000 * (1/bucket_size));
                // int energy_harvested_buckets = 1 + static_cast<int>(std::floor((energy_harvested + braking_energy) / 1000000 * (1/bucket_size)));

                // // Option table generating loop, doesn't need to harvest all the energy given
                // for (int energy = 0; energy < energy_harvested_buckets; energy++){
                //     const double energy_bucket_MJ = energy * bucket_size;

                //     Option temp = {.deploy = energy_deployed, .harvest = energy_bucket_MJ, .delta = total_time};
                //     ExecutionDetails exe = {.deployment_distance = deploy_dis, .deployment_rate = deploy_rate, .harvest_distance = harvest_dis,
                //                             .harvest_rate = harvest_rate, .braking_distance = braking_dis};
                //     output.push_back(temp);
                //     execution_table.push_back(exe);
                // }
            }
        }
    }
    execution_lookup_table[seg_index] = execution_table;
    return output;
}

void Optimizer::initialize_option_table_lookup_table(){
    option_table_lookup_table.resize(circuit.size());

    for(int i = 0; i< circuit.size(); ++i){
        option_table_lookup_table[i] = segment_options(i);
    }
}
