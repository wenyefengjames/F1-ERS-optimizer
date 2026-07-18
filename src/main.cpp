#include <iostream>
#include "../include/optimizer.h"
#include <string>

int main() {
    
    std::string mode_input;
    std::string mom_input;
    bool race_mode = false;
    bool mom = false;
    double laptime = 0.0;

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

    Optimizer ems(race_mode, mom);
    if(race_mode){ // long distance
        laptime =  ems.main_optimizing_loop(0, 4.0, 3.5);
    }
    else{ // qualifying
        laptime = ems.main_optimizing_loop(0, 4.0, 0);
    }

    std::cout << "Best lap time: " << laptime << '\n';

    return 0;
}