#include <iostream>
#include "../include/optimizer.h"
#include <string>

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

    // std::cout << "ERS Optimizer Program" << std::endl;
    // std::cout << "Enter 'race' or 'qualify' to choose mode: ";
    // std::cin >> mode_input;
    // std::cout << "Within 1 second of the car ahead?: ";
    // std::cin >> mom_input;

    // if(mode_input == "race"){
    //     race_mode = true;
    // }
    // if(mom_input == "yes"){
    //     mom = true;
    // }

    // std::cout << "Starting battery: ";
    // std::cin >> start_bat;
    // std::cout << "Ending battery: ";
    // std::cin >> end_bat;
    // std::cout << "Starting harvest: ";
    // std::cin >> harvest;
    // std::cout << "Starting segment: ";
    // std::cin >> seg_index;

    Optimizer ems(race_mode, mom);
    // if(race_mode){ // long distance
    //     laptime =  ems.main_optimizing_loop(seg_index, start_bat, end_bat, harvest);
    // }
    // else{ // qualifying
    //     laptime = ems.main_optimizing_loop(0, 4.0, 0, 0);
    // }

    laptime = ems.main_optimizing_loop(0, 4.0, 0, 0);

    std::cout << "Lap time: 1." << (laptime - 60) << '\n';

    return 0;
}

// int main(){
//     bool race_mode = true;
//     bool mom = true;
//     Optimizer ems(race_mode, mom);

//     double time = 0;

//     // for(int i = 0; i < ems.circuit.size(); i++){

//     //     int count = 1;
//     //     std::vector<Option> table = ems.segment_options(i, 3);
//     //     std::cout << "Segment name: " << ems.circuit.at(i)->get_name() << '\n';
//     //     std::cout << "==================" << '\n';

//         // All corners
//         // if(i != 0 && i != 6 && i != 9 && i != 11 && i != 18 && i != 16){
//         //     std::vector<Option> table = ems.segment_options(i, 3);
//         //     std::cout << "Segment name: " << ems.circuit.at(i)->get_name() << '\n';
//         //     std::cout << "==================" << '\n';
//         //     for(const Option& row : table){
//         //         std::cout << "Deploy: " << row.deploy << '\t';
//         //         std::cout << "Harvest: " << row.harvest << '\t';
//         //         std::cout << "delta: " << row.delta << '\n';
//         //     }
//         // }

//         // All Straghts
//         // if(i == 0 || i == 6 || i == 9 || i == 11 || i == 18 || i == 16){
//         //     std::cout << "Segment name: " << ems.circuit.at(i)->get_name() << '\n';
//         //     std::cout << "==================" << '\n';
//         //     for(const Option& row : table){

//         //         if(count % 6 == 0){
//         //             std::cout << "Deploy: " << row.deploy << '\t';
//         //             std::cout << "Harvest: " << row.harvest << '\t';
//         //             std::cout << "delta: " << row.delta << '\n';
//         //             count = 1;
//         //         }
//         //         else count += 1;
//         //     }
//         // }


//         // std::cout << "Longest time: " << table[0].delta << '\n';
//         // time += table[0].delta;
//     // }

//     double initial_bat = 3;
    
//     std::vector<Option> straight_table = ems.option_table_straight(0, initial_bat);
//     // std::vector<Option> fast_corner_table = ems.option_table_fastcorner(5, initial_bat);
//     // std::vector<Option> slow_corner_table = ems.option_table_slowcorner(4);

//     std::cout << "Straight table" << '\n';

//     for(const Option& row : straight_table){
//         std::cout << "Deploy: " << row.deploy << '\t';
//         std::cout << "Harvest: " << row.harvest << '\t';
//         std::cout << "delta: " << row.delta << '\n';
//     }

//     // std::cout << "Fast corner table" << '\n';

//     // for(const Option& row : fast_corner_table){
//     //     std::cout << "Deploy: " << row.deploy << '\t';
//     //     std::cout << "Harvest: " << row.harvest << '\t';
//     //     std::cout << "delta: " << row.delta << '\n';
//     // }

//     // std::cout << "Slow corner table" << '\n';
    
//     // for(const Option& row : slow_corner_table){
//     //     std::cout << "Deploy: " << row.deploy << '\t';
//     //     std::cout << "Harvest: " << row.harvest << '\t';
//     //     std::cout << "delta: " << row.delta << '\n';
//     // }

//     return 0;
// }