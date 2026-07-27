#include "../include/physics.h"
#include <iostream>

namespace p = physics;

int main(){

    double speed = 300;
    double distance = 300;
    bool mom = false;

    TaperedDeploymentResult output = p::energy_deployed_with_taper(speed, distance, mom);
    double energy = p::work_done_with_drag(750, speed, distance);
    double no_speed = p::reverse_ke(speed,energy);
    double time = p::time_to_reach_velocity(no_speed, speed, 750);

    std::cout << "Speed: " << output.speed_kmh << "\t";
    std::cout << "Speed without taper: " << no_speed << "\n";
    std::cout << "Time: " << output.time_s << "\t";
    std::cout << "Time without taper: " << time << "\n";
    std::cout << "Dis: " << output.distance_m << "\t";
    std::cout << "Dis:  without taper: " << distance << "\n";
    std::cout << "Energy: " << p::kinetic_energy(output.speed_kmh) - p::kinetic_energy(speed) << "\t";
    std::cout << "Energy without taper: " << energy << "\n";

    double power = p::required_power(speed, 0, 200);
    std::cout << "======================" << "\n";
    std::cout << "Power output required" << power << "\n";

}