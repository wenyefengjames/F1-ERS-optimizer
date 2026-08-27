#include <iostream>
#include "../include/optimizer.h"
#include <string>

// Temporarily disabled -- diagnostic main() below is investigating why the DP is slow.
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

    }
    else{
        race_mode = false;
        Optimizer ems(race_mode, mom);
        laptime = ems.main_optimizing_loop(0, 4.0, 0, 0);
    }

    // laptime = ems.main_optimizing_loop(0, 4.0, 0, 0);

    std::cout << "Lap time: 1." << (laptime - 60) << '\n';

    return 0;
}

// // Diagnostic: dump every segment's option table (sampled every 10th entry once a table
// // gets large) plus a per-segment and grand total count, to see how big the DP's actual
// // search space is -- state-space size is the first thing to check when the cached
// // tables build fast but dp_algorithm() itself is slow.
// int main(){
//     bool race_mode = false;
//     bool mom = true;
//     Optimizer ems(race_mode, mom);

//     long long total_options = 0;

//     for(int i = 0; i < ems.circuit.size(); i++){
//         std::vector<Option> table = ems.segment_options(i);

//         std::cout << "===========================================\n";
//         std::cout << "Segment " << i << ": " << ems.circuit.at(i)->get_name() << "\n";
//         std::cout << "-------------------------------------------\n";

//         // Print every entry for small tables; sample every 10th once it gets large,
//         // so a table with thousands of options doesn't flood the console.
//         size_t step = (table.size() > 20) ? 10 : 1;

//         // for(size_t j = 0; j < table.size(); j += step){
//         //     const Option& op = table[j];
//         //     std::cout << "  [" << j << "] Deploy: " << op.deploy << " MJ\t";
//         //     std::cout << "Harvest: " << op.harvest << " MJ\t";
//         //     std::cout << "Delta: " << op.delta << " s\n";
//         // }

//         std::cout << "Total options for this segment: " << table.size() << "\n";
//         total_options += static_cast<long long>(table.size());
//     }

//     std::cout << "===========================================\n";
//     std::cout << "Total options across all segments: " << total_options << "\n";

//     return 0;
// }
