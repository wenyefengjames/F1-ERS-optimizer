#include "../include/physics.h"
#include <iostream>
#include <optional>
#include <limits>
#include <cmath>

namespace p = physics;
using namespace std;

// int main(){

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

    // double initial_speed = 270.005;
    // double final_speed = 270;
    // double distance = 100;
    // bool mom = false;
    // bool sm_on = true;

    // int partition_size = 50;

    // cout << "final_speed: " << final_speed << '\n';
    // for(int i = 0; i < partition_size; i++){
    //     double speed = initial_speed - i * ((initial_speed - final_speed) / partition_size);

    //     double ke_diff = p::kinetic_energy(final_speed) - p::kinetic_energy(speed);
    //     double power = p::required_power(speed, ke_diff, distance, mom, sm_on);
    //     double time = p::time_to_reach_velocity(final_speed, speed, power / 1000, sm_on);

    //     cout << "Power: " << power << '\t';
    //     cout << "Time: " << time << '\t';
    //     cout << "Initial speed: " << speed << '\n';
    // }

    // std::vector<TaperedDeploymentResult> results = p::taper_table(mom, sm_on);

    // for(const auto& i: results){
    //     std::cout << "Speed: " << i.speed_kmh << "     ";
    //     std::cout << "Time: " << i.time_s << "     ";
    //     std::cout << "Distance: " << i.distance_m << "     ";
    //     std::cout << "Energy: " << i.energy_J << "\n";
    // }

    // double query = 295;
    // auto lambda_func = [](const TaperedDeploymentResult& field) {return field.speed_kmh;};

    // std::optional<TaperedDeploymentResult> search_result = p::search_taper_table(mom, sm_on, query, lambda_func);

    // TaperedDeploymentResult result = search_result.value();

    // std::cout << "Speed: " << result.speed_kmh << "     ";
    // std::cout << "Time: " << result.time_s << "     ";
    // std::cout << "Distance: " << result.distance_m << "     ";
    // std::cout << "Energy: " << result.energy_J << "\n";

    // ================================================================
    // Diagnostic sweep #1 (superseded -- refuted the "deeper into taper"
    // hypothesis, kept for reference): for each straight, deploy over its
    // full length from its real entry speed and see how far past the 290
    // km/h taper threshold the final speed lands.
    // StraightCheck straights[] = {
    //     {"Hamilton",   243.0, 410.0,   25.0,  325.0},
    //     {"Wellington", 257.0, 585.0,    0.0,  585.0},
    //     {"Woodcote",   230.0, 690.0,    0.0,  590.0},
    //     {"Hanger",     244.0, 813.5,   90.5,  813.5},
    // };

    // for(const auto& s : straights){
    //     TaperedDeploymentResult r = p::energy_deployed_with_taper(s.entry_speed, s.length, s.sm_start, s.sm_end, false);

    //     cout << s.name << ":\n";
    //     cout << "  final speed_kmh: " << r.speed_kmh << "\t(gap above 290: " << r.speed_kmh - 290.0 << ")\n";
    //     cout << "  time_s: " << r.time_s << "\n";
    //     cout << "  distance_m: " << r.distance_m << " (requested " << s.length << ")\n";
    //     cout << "  energy_J: " << r.energy_J << "\n\n";
    // }
// }

