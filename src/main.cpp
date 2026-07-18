#include <iostream>
#include <optimizer.h>

int main() {
    
    std::string mode_input;
    std::string mom_input;
    bool race_mode = false;
    bool mom = false;

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

    Optimizer ems = Optimizer(race_mode, mom);
    if(race_mode){ // long distance
        ems.main_optimizing_loop(0.0, 4.0, 4.0);
    }
    else{ // qualifying
        ems.main_optimizing_loop(0.0, 4.0, 0);
    }

    return 0;
}