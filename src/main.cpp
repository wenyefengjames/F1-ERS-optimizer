#include <iostream>
#include "../include/optimizer.h"
#include <string>

int main() {
    
    std::string mode_input;
    std::string mom_input;
    bool race_mode = false;
    bool mom = false;
    double laptime = 0.0;
    double start_bat = 0.0;
    double end_bat = 0.0;
    double harvest = 0.0;
    int seg_index = 0;

    std::cout << "ERS Optimizer Program" << std::endl;
    std::cout << "Enter 'race' or 'qualify' to choose mode: ";
    std::cin >> mode_input;
    std::cout << "Within 1 second of the car ahead?: ";
    std::cin >> mom_input;

    if(mode_input == "race"){
        race_mode = true;
    }
    if(mom_input == "yes"){
        mom = true;
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
    if(race_mode){ // long distance
        laptime =  ems.main_optimizing_loop(seg_index, start_bat, end_bat, harvest);
    }
    else{ // qualifying
        laptime = ems.main_optimizing_loop(0, 4.0, 0, 0);
    }

    std::cout << "Lap time: 1." << (laptime - 60) << '\n';

    return 0;
}