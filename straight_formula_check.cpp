// Quick numeric check of the Straight-segment formula chain in
// Optimizer::segment_options(), using real Vale-straight data (a case where
// target_speed < exit_speed, i.e. the harvesting/deceleration branch
// actually gets exercised). Not part of the CMake build -- compile/run ad hoc:
//
//   g++ -std=c++20 -Iinclude straight_formula_check.cpp src/physics.cpp -o straight_formula_check
//   ./straight_formula_check

#include "./include/physics.h"
#include <iostream>
#include <cmath>

int main(){
    // Vale straight: length 286m, prev = Stowe (apex_min_speed 236), next = Turn 16-18 (apex_min_speed 102)
    double exit_speed = 236.0;
    double target_speed = 300;
    int length = 450;
    int partition_size = 15;
    int dis = 18; // representative middle-ish partition index
    int deploy_dis = dis * partition_size;
    int harvest_dis = length - deploy_dis;

    std::cout << "deploy_dis=" << deploy_dis << " harvest_dis=" << harvest_dis << "\n\n";

    double ke_gained = physics::work_done_with_drag(physics::MGU_K + physics::ICE, exit_speed, deploy_dis);
    std::cout << "ke_gained = " << ke_gained << " J\n";

    double speed = physics::reverse_ke(exit_speed, ke_gained);
    std::cout << "speed after deploy = " << speed << " km/h\n";

    double time_deploying = physics::time_to_reach_velocity(speed, exit_speed, physics::MGU_K + physics::ICE);
    std::cout << "time_deploying = " << time_deploying << " s\n";

    double energy_deployed_NO_CONVERSION = time_deploying * physics::MGU_K;
    double energy_deployed_WITH_CONVERSION = time_deploying * physics::MGU_K * 1000.0;
    std::cout << "energy_deployed (as written, kW*s)   = " << energy_deployed_NO_CONVERSION << "\n";
    std::cout << "energy_deployed (x1000 -> Joules)      = " << energy_deployed_WITH_CONVERSION << "\n\n";

    double ke_diff = physics::kinetic_energy(speed) - physics::kinetic_energy(target_speed);
    std::cout << "ke_diff = " << ke_diff << " J\n";

    double engine_power = physics::required_power(speed, -ke_diff, harvest_dis) / 1000;
    std::cout << "engine_power (required_power result) = " << engine_power << " kW\n";

    double time_harvesting = physics::time_to_reach_velocity(target_speed, speed, engine_power);
    std::cout << "time_harvesting = " << time_harvesting << " s  " << (std::isnan(time_harvesting) ? "<<<< NaN!" : "") << "\n";

    double energy_harvested_NO_CONVERSION = time_harvesting * (physics::ICE - engine_power);
    double energy_harvested_WITH_CONVERSION = time_harvesting * (physics::ICE - engine_power) * 1000.0;
    std::cout << "energy_harvested (as written)   = " << energy_harvested_NO_CONVERSION << "kJ\n";
    std::cout << "energy_harvested (x1000)        = " << energy_harvested_WITH_CONVERSION << "J\n";

    return 0;
}