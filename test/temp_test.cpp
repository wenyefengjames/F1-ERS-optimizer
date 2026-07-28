#include "../include/physics.h"
#include <iostream>
#include <optional>

namespace p = physics;

int main(){

    double speed = 300;
    double distance = 300;
    

    // TaperedDeploymentResult output = p::energy_deployed_with_taper(speed, distance, mom);
    // double energy = p::work_done_with_drag(750, speed, distance);
    // double no_speed = p::reverse_ke(speed,energy);
    // double time = p::time_to_reach_velocity(no_speed, speed, 750);

    // std::cout << "Speed: " << output.speed_kmh << "\t";
    // std::cout << "Speed without taper: " << no_speed << "\n";
    // std::cout << "Time: " << output.time_s << "\t";
    // std::cout << "Time without taper: " << time << "\n";
    // std::cout << "Dis: " << output.distance_m << "\t";
    // std::cout << "Dis:  without taper: " << distance << "\n";
    // std::cout << "Energy: " << p::kinetic_energy(output.speed_kmh) - p::kinetic_energy(speed) << "\t";
    // std::cout << "Energy without taper: " << energy << "\n";

    // double power = p::required_power(speed, 0, 200, mom);
    // std::cout << "======================" << "\n";
    // std::cout << "Power output required" << power << "\n";

    bool mom = false;
    bool sm_on = true;

    std::vector<TaperedDeploymentResult> results = p::taper_table(mom, sm_on);

    for(const auto& i: results){
        std::cout << "Speed: " << i.speed_kmh << "     ";
        std::cout << "Time: " << i.time_s << "     ";
        std::cout << "Distance: " << i.distance_m << "     ";
        std::cout << "Energy: " << i.energy_J << "\n";
    }

    double query = 295;
    auto lambda_func = [](const TaperedDeploymentResult& field) {return field.speed_kmh;};

    std::optional<TaperedDeploymentResult> search_result = p::search_taper_table(mom, sm_on, query, lambda_func);

    TaperedDeploymentResult result = search_result.value();

    std::cout << "Speed: " << result.speed_kmh << "     ";
    std::cout << "Time: " << result.time_s << "     ";
    std::cout << "Distance: " << result.distance_m << "     ";
    std::cout << "Energy: " << result.energy_J << "\n";
}