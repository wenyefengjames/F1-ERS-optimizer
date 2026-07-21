#include "../include/optimizer.h"
#include <cmath>
#include <limits>
#include <algorithm>
#include <iostream>

namespace p = physics;

Optimizer::Optimizer(bool race_mode, bool mom) :
                     race_mode(race_mode), mom(mom){
    table.resize(circuit.size() * battery_buckets * battery_buckets * harvest_buckets, -1.0);
    choice.resize(circuit.size() * battery_buckets * battery_buckets * harvest_buckets, std::nullopt);
}

// i for index: which segment are we currently in
// b for battery level: since it is a stepping of 0.1 between 0-4. We will multiply this by 10 to give int
// e for ending battery level: same as b
// h for harvest: to stop the algorithm from reaching over the harvest limit
int Optimizer::index_helper(int i, double b, double e, double h){
    int b_bucket = static_cast<int>(std::round(b * (1/bucket_size)));
    int e_bucket = static_cast<int>(std::round(e * (1/bucket_size)));
    int h_bucket = static_cast<int>(std::round(h * (1/bucket_size)));

    int value = h_bucket * (circuit.size() * battery_buckets * battery_buckets) + 
                (e_bucket * circuit.size() * battery_buckets + (i * battery_buckets + b_bucket));
    return value;
}

// Input: seg_index, the index of the segment of the track that you want to start the simulation on
//        initial_battery, the starting battery that you want to give the car, in MJ
//        ending_battery, how much leftover battery charge you need, in MJ
//        harvest, the starting harvest that you want to give the car, should be 0 by default
double Optimizer::main_optimizing_loop(int seg_index, double initial_battery, double ending_battery, double harvest){
    Battery battery = Battery(initial_battery, harvest, race_mode);
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
        std::cout << "Deployed: " << op.deploy << "MJ \t";
        std::cout << "Harvested: " << op.harvest << "MJ \t";
        std::cout << "Time spent: " << op.delta << '\n';
        // std::cout << "battery before harvest: " << battery.get_harvest_charge() << '\t';
        // std::cout << "uncapped harvest: " << battery.get_harvest_charge() + op.harvest << '\t';
        // std::cout << "Is harvest full?: " << (battery.get_harvest_charge() + op.harvest 
        //                                     <= battery.get_harvest_limit()) << '\n';


        // Calculate sector times
        if(seg_index >=0 && seg_index <= 4){
            sector_1 += op.delta;
        }
        else if (seg_index >=5 && seg_index <= 11){
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

    int i = index_helper(index, battery.get_battery_charge(), ending_battery, battery.get_harvest_charge());

    // Return value immediately if there is a memoization of the current state
    if (table.at(i) != -1){
        // TESTING: SHOULD BE REMOVED AFTER TESTING IS COMPLETE
        // std::cout << "apprently I found something already: " << table.at(i) << '\n';
        return table.at(i);
    }

    std::optional<Option> best_path;
    double best_time = std::numeric_limits<double>::infinity();
    double total_time = std::numeric_limits<double>::infinity();
    double remaining_time = 0.0;
    
    // This should be a vector of Option struct that gives all the strategy options for
    // the current segment of the race track that we are on. 
    std::vector<Option> current_segment = segment_options(index, battery.get_battery_charge());
    
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
        remaining_time = 0.0;

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
        // std::cout << " check allow charge: " << battery.check_allow_charge(option.deploy, option.harvest) << '\t';
        // std::cout << "Check if-statement: " << (battery.check_allow_charge(option.deploy, option.harvest) && ending_battery_ok) << '\n';

        if (battery.check_allow_charge(option.deploy, option.harvest) && ending_battery_ok){

            Battery next_battery = battery;
            next_battery.deploy(option.deploy);
            next_battery.harvest(option.harvest);

            // std::cout << "current segment name: " <<  circuit.at(index)->get_name() << '\n';
            // std::cout << "next segment name: " <<  circuit.next(index)->get_name() << '\n';

            remaining_time = dp_algorithm(index + 1, next_battery, ending_battery);
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
    int index = 0;

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
std::vector<Option> Optimizer::segment_options(int seg_index, double initial_battery){
    std::vector<Option> option_table;    

    if (circuit.at(seg_index)->get_type() == "SlowCorner"){

        return option_table_slowcorner(seg_index);
    } 
    else if(circuit.at(seg_index)->get_type() == "FastCorner"){

        return option_table_fastcorner(seg_index, initial_battery); 
    } 
    else if(circuit.at(seg_index)->get_type() == "Straight"){

        return option_table_straight(seg_index, initial_battery);      
    }
    return option_table;
}

// Produces the option table for a fast corner
// The output should only have one entry, the energy can be both deploy or harvest
std::vector<Option> Optimizer::option_table_fastcorner(int seg_index, double initial_battery){

    // No type check, because this function should only be called on a fast corner
    FastCorner* corner = static_cast<FastCorner*>(circuit.at(seg_index));
    std::vector<Option> option_table;
    double current_speed = corner->get_apex_min_speed();
    double target_speed = 0.0;
    double length = corner->get_length() * 0.5;
    
    // Extract the target speed and length of the next segment, fast and slow corners are different here
    Segment* next_segment = circuit.next(seg_index);
    if(next_segment->get_type() == "SlowCorner") {
        SlowCorner* next_corner = static_cast<SlowCorner*>(next_segment);
        target_speed = next_corner->get_entry_speed();
    }
    else if(next_segment->get_type() == "FastCorner"){
        FastCorner* next_corner = static_cast<FastCorner*>(next_segment);
        target_speed = next_corner->get_apex_min_speed();

        // Assumption that the apex of the next corner is at exactly half way of the corner
        // Which is where the apex_min_speed lies
        length += next_corner->get_length() * 0.5;
    }
    else if (next_segment->get_type() == "Straight") {
        target_speed = corner->get_exit_speed();
    }

    const double ke_gain = p::kinetic_energy(target_speed) - p::kinetic_energy(current_speed);
    const double power_output = p::required_power(current_speed, ke_gain, length) / 1000;

    // TESTING: SHOULD BE REMOVED AFTER TESTING IS COMPLETE
    // std::cout << "ke_gain: " << ke_gain << "\t";
    // std::cout << "power_output: " << power_output << "\t";
    // std::cout << "length: " << length << "\t";
    // std::cout << "init battery: " << initial_battery << "\n";

    // Indicates that the car needs braking, which should never happen
    if (power_output < 0){
        option_table.clear();
        return option_table;
    }

    const double time = p::time_to_reach_velocity(target_speed, current_speed, power_output);

    const double recharge_rating = std::clamp(p::ICE - power_output, -p::MGU_K, p::MGU_K);

    double harvest_energy_MJ = recharge_rating * 1000 * time / 1000000;

    // TESTING: SHOULD BE REMOVED AFTER TESTING IS COMPLETE
    // std::cout << "initial_battery < -harvest_energy_MJ: " << (initial_battery < -harvest_energy_MJ) << "\t";
    // std::cout << "time: " << time << "\t";
    // std::cout << "harvest_energy_MJ: " << harvest_energy_MJ << "\n";

    // The amount of energy in battery can be less than what we need
    // TO DO. Deal with this later because this will affect the fixed exit speed
    // For now we set the time to infinity
    if(initial_battery < -harvest_energy_MJ){
        option_table.clear();
    }
    else{
        if(harvest_energy_MJ >= 0){ // Actually harvesting
            double energy_buckets = std::floor(harvest_energy_MJ * (1/bucket_size)) + 1;

            // Generate a list of allowed energy to harvest, the time would be the same
            for(int step = 0; step < energy_buckets; step++){
                Option temp = {0, step * bucket_size, time};
                option_table.push_back(temp);
            }

        }
        else{   // Deploying
            harvest_energy_MJ = bucket_size * std::ceil(-harvest_energy_MJ * (1/bucket_size));
            Option temp = {harvest_energy_MJ, 0, time};
            option_table.push_back(temp);
        }
    }

    return option_table;
}

std::vector<Option> Optimizer::option_table_slowcorner(int seg_index){
    
    // No type check here, because this function should only be called for a slow corner
    SlowCorner* corner = static_cast<SlowCorner*>(circuit.at(seg_index));
    std::vector<Option> option_table;

    const double entry_speed = corner->get_entry_speed();
    const double min_speed = corner->get_apex_min_speed();
    const double corner_duration = corner->get_time();
    const double throttle_percentage = corner->get_throttle_percentage() * 0.01;

    const double braking_duration = (entry_speed - min_speed) / (p::BRAKING_DECEL * 3.6);
    const double braking_energy = braking_duration * p::MGU_K * 1000;
    const double partial_throttle_duration = corner_duration - braking_duration;
    double recharge_rating = (1.0 - throttle_percentage) * p::ICE;

    // Cap the recharge rate, in kW
    if (recharge_rating >= p::MGU_K) recharge_rating = p::MGU_K;

    const double partial_throttle_energy = partial_throttle_duration * recharge_rating * 1000;
    const double harvest_energy = partial_throttle_energy + braking_energy;

    // Capped at the total harvestable energy, not allowed to deploy
    const int bucket_num = std::floor(harvest_energy / (bucket_size * 1000000) + 1);

    // Option table generating loop
    for (int energy = 0; energy < bucket_num; energy++){
        const double energy_bucket_MJ = energy * bucket_size;

        // The delta is invariant to deployment, can't deploy energy as grip is limitation
        Option temp = {0.0, energy_bucket_MJ, corner_duration};
        option_table.push_back(temp);
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

    // TESTING
    // std::cout << "exit speed: " << exit_speed << "\t";
    // std::cout << "target speed: " << target_speed << "\t";
    // std::cout << "length: " << length << "\t";
    // std::cout << "init battery: " << initial_battery << "\n";

    return best_option_for_bucket(length, exit_speed, target_speed, initial_battery);
}

// Loop through the optimal energy deployment method for every partition_size meter gap
std::vector<Option> Optimizer::best_option_for_bucket(int length, double exit_speed, double target_speed,
                                                      double initial_battery){

    double best_time = std::numeric_limits<double>::infinity();
    double total_time = std::numeric_limits<double>::infinity();
    double deploy_choice = 0.0;     // Energy deployed for the best time
    double harvest_choice = 0.0;    // Energy harvested for the best time
    const double ke_target_speed = p::kinetic_energy(target_speed); // loop invariant, so precompute
    const int full_engine_power = p::MGU_K + p::ICE; // loop invariant

    std::vector<Option> output;

    // Find the optimal time for the given energy bucket
    for (int dis = 0; dis < length / partition_size; dis++){
        total_time = std::numeric_limits<double>::infinity();

        const double deploy_dis = dis * partition_size;
        const double harvest_dis = length - deploy_dis;

        // TESTING
        // std::cout << "deploy_dis: " << deploy_dis << "\t";
        // std::cout << "harvest_dis: " << harvest_dis << "\n";

        const double ke_gained = p::work_done_with_drag(full_engine_power, exit_speed, deploy_dis);
        const double speed = p::reverse_ke(exit_speed, ke_gained);
        const double time_deploying = p::time_to_reach_velocity(speed, exit_speed, full_engine_power);

        double energy_deployed = time_deploying * p::MGU_K * 1000;

        // TESTING
        // std::cout << "energy_deployed: " << energy_deployed << "\t";
        // std::cout << "ke_gained: " << ke_gained << "\t";
        // std::cout << "speed: " << speed << "\t";
        // std::cout << "time_deploying: " << time_deploying << "\n";

        // If the amount of energy deployed is more than what the battery have, break out of this loop
        // Because the deployment distance will only keep increasing, so the battery wont have enough for distance longer than current
        if(energy_deployed >= initial_battery * 1000000){
            break;
        }

        const double ke_diff = p::kinetic_energy(speed) - ke_target_speed;
        const double engine_power = p::required_power(speed, -ke_diff, harvest_dis);   // in Watts, not kW

        // TESTING
        // std::cout << "ke_diff: " << ke_diff << "\t";
        // std::cout << "engine_power: " << engine_power << "\n";
                    
        // Stop the loop if it requires the engine power output to be less than 0 (meaning it requires breaking)
        if(engine_power < 0){
            break;
        }

        // Limit the range of recharge
        double recharge_power = p::ICE*1000 - engine_power;
        
        // Recharge power has to be positive
        if (recharge_power <= 0){
            continue;
        }

        if (recharge_power >= p::MGU_K * 1000) recharge_power = p::MGU_K * 1000;

        const double time_harvesting = p::time_to_reach_velocity(target_speed, speed, engine_power / 1000);

        double energy_harvested = time_harvesting * recharge_power;
        total_time = time_harvesting + time_deploying;

        // TESTING
        // std::cout << "energy_harvested: " << energy_harvested << "\t";
        // std::cout << "time_harvesting: " << time_harvesting << "\n";
        // std::cout << "total energy: " << energy_harvested - energy_deployed << "\t";
        // std::cout << "total time: " << total_time << "\n";
        // std::cout << "============================ " << "\n";

        energy_deployed = bucket_size * std::ceil(energy_deployed  / 1000000 * (1/bucket_size));
        int energy_harvested_buckets = 1 + std::floor(energy_harvested / 1000000 * (1/bucket_size));

        // Option table generating loop, doesn't need to harvest all the energy given
        for (int energy = 0; energy < energy_harvested_buckets; energy++){
            const double energy_bucket_MJ = energy * bucket_size;

            Option temp = {energy_deployed, energy_bucket_MJ, total_time};
            output.push_back(temp);
        }
    }
    return output;
}

