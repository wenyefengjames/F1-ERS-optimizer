#include <iostream>
#include "../include/optimizer.h"
#include <string>

// Temporarily disabled -- diagnostic main() below investigates why Hamilton
// Straight's chosen option reports minimal MGU-K deployment.

int main() {

    std::string mode_input;
    std::string mom_input;
    bool race_mode = false;
    bool mom = true;
    double laptime = 0.0;
    double start_bat = 0.0;
    double end_bat = 0.0;
    double harvest = 0.0;
    int seg_index = 0;

    std::cout << "ERS Optimizer Program" << std::endl;
    std::cout << "Enter 'race' or 'qualify' to choose mode: ";
    std::cin >> mode_input;

    if(mode_input == "race"){
        race_mode = true;
        std::cout << "Within 1 second of the car ahead?: ";
        std::cin >> mom_input;

        if(mom_input == "yes"){
            mom = true;
        }
        else{
            mom = false;
        }

        std::cout << "Starting battery: ";
        std::cin >> start_bat;
        std::cout << "Ending battery: ";
        std::cin >> end_bat;
        std::cout << "Starting harvest: ";
        std::cin >> harvest;
        std::cout << "Starting segment: ";
        std::cin >> seg_index;

        Optimizer ems(race_mode, mom);
        laptime =  ems.main_optimizing_loop(seg_index, start_bat, end_bat, harvest);
        // NEW: writes the winning path's reconstructed speed trace for plotting against Kimi's lap.
        ems.write_speed_trace_csv("battery_deployment_silverstone.csv", seg_index, start_bat, end_bat, harvest);

    }
    else{
        race_mode = false;
        Optimizer ems(race_mode, mom);
        laptime = ems.main_optimizing_loop(0, 4.0, 0, 0);
        // NEW: writes the winning path's reconstructed speed trace for plotting against Kimi's lap.
        ems.write_speed_trace_csv("battery_deployment_silverstone.csv", 0, 4.0, 0, 0);
    }

    // laptime = ems.main_optimizing_loop(0, 4.0, 0, 0);

    std::cout << "Lap time: 1." << (laptime - 60) << '\n';

    return 0;
}


// Diagnostic: dump every option generated for Hamilton Straight (segment 0),
// paired with its ExecutionDetails, to check whether the chosen option's
// minimal MGU-K deploy figure is explained by a short deployment distance
// with the ICE baseline carrying most of the acceleration.
// int main(){
//     bool race_mode = false;
//     bool mom = true;
//     Optimizer ems(race_mode, mom);

//     const int seg_index = 0;
//     Segment* seg = ems.circuit.at(seg_index);
//     Corner* next_corner = static_cast<Corner*>(ems.circuit.next(seg_index));

//     std::vector<Option> options = ems.option_table_straight(seg_index);
//     const std::vector<ExecutionDetails>& executions = ems.execution_lookup_table[seg_index];

//     std::cout << "Total options generated: " << options.size() << "\n";
//     std::cout << "===========================================\n";

//     for(size_t j = 0; j < options.size(); j++){
//         const Option& op = options[j];
//         const ExecutionDetails& exe = executions[j];

//         std::cout << "[" << j << "] "
//                   << "Deploy: " << op.deploy << "MJ\t"
//                   << "Harvest: " << op.harvest << "MJ\t"
//                   << "Delta: " << op.delta << "s"
//                   << " || DeployDis: " << exe.deployment_distance_m << "m"
//                   << " @ " << exe.deployment_rate_kW << "kW\t"
//                   << "HarvestDis: " << exe.harvest_distance_m << "m"
//                   << " @ " << exe.harvest_rate_kW << "kW\t"
//                   << "BrakingDis: " << exe.braking_distance_m << "m\n";
//     }

//     std::cout << "===========================================\n";
//     std::cout << "Segment: " << seg->get_name() << "\n";
//     std::cout << "Length: " << seg->get_length() << " m\n";
//     std::cout << "Starting speed (hardcoded fallback, prev segment is a Straight): 245 km/h\n";
//     std::cout << "Ending speed (T1-2 entry speed, real): " << next_corner->get_entry_speed() << " km/h\n";

//     return 0;
// }
