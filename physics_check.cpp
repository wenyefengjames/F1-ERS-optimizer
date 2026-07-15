// Quick, framework-free validity check for physics.cpp -- the C++ equivalent
// of manually poking values into test.py's REPL and eyeballing the output,
// but fixed and repeatable. Not part of the CMake build; compile/run ad hoc:
//
//   g++ -std=c++20 -Iinclude physics_check.cpp src/physics.cpp -o physics_check
//   ./physics_check
//
// A real unit test suite (Catch2/Google Test, TBD) is still deferred to a
// later phase -- this only guards against the formulas silently breaking.

#include "include/physics.h"
#include <cmath>
#include <iostream>
#include <string>

namespace {

int total_checks = 0;
int passed_checks = 0;

bool check(const std::string& name, double actual, double expected, double tol = 1e-4){
    total_checks++;
    double diff = std::fabs(actual - expected);
    double scale = std::max(1.0, std::fabs(expected));
    bool pass = diff <= tol * scale;

    std::cout << (pass ? "PASS" : "FAIL") << " - " << name;
    if(!pass){
        std::cout << "  (expected " << expected << ", got " << actual << ", diff " << diff << ")";
    }
    std::cout << "\n";

    if(pass) passed_checks++;
    return pass;
}

bool check_true(const std::string& name, bool condition){
    total_checks++;
    std::cout << (condition ? "PASS" : "FAIL") << " - " << name << "\n";
    if(condition) passed_checks++;
    return condition;
}

}

int main(){
    // -- kinetic_energy -----------------------------------------------
    check("kinetic_energy(0) is 0", physics::kinetic_energy(0.0), 0.0);

    {
        double speed_kmh = 200.0;
        double speed_ms = speed_kmh / 3.6;
        double expected = 0.5 * physics::MASS_KG * speed_ms * speed_ms;
        check("kinetic_energy(200) matches hand-computed 0.5*m*v^2", physics::kinetic_energy(speed_kmh), expected);
    }

    // -- work_done_with_drag --------------------------------------------
    check("work_done_with_drag(...) is 0 at distance 0", physics::work_done_with_drag(300.0, 150.0, 0.0), 0.0);

    {
        double e_100m = physics::work_done_with_drag(300.0, 150.0, 100.0);
        double e_200m = physics::work_done_with_drag(300.0, 150.0, 200.0);
        check_true("work_done_with_drag increases with distance", e_200m > e_100m);
    }

    // -- required_power: round-trips through work_done_with_drag --------
    {
        double vi = 150.0, x = 100.0, power_kW = 300.0;
        double energy_target = physics::work_done_with_drag(power_kW, vi, x);

        double required_power_W = physics::required_power(vi, energy_target, x);
        check("required_power recovers the original power_kW", required_power_W / 1000.0, power_kW);

        double energy_check = physics::work_done_with_drag(required_power_W / 1000.0, vi, x);
        check("required_power round-trips through work_done_with_drag", energy_check, energy_target);
    }

    // -- distance_to_recharge: round-trips through work_done_with_drag ---
    // This is the check that would have caught the (-mass/3*k) parenthesization bug.
    {
        double vi = 150.0, x = 100.0, power_kW = 300.0;
        double energy_target = physics::work_done_with_drag(power_kW, vi, x);

        double recovered_distance = physics::distance_to_recharge(vi, energy_target, power_kW);
        check("distance_to_recharge recovers the original distance", recovered_distance, x);

        double energy_check = physics::work_done_with_drag(power_kW, vi, recovered_distance);
        check("distance_to_recharge round-trips through work_done_with_drag", energy_check, energy_target);
    }

    // -- coasting_energy_loss --------------------------------------------
    check("coasting_energy_loss is 0 at distance 0", physics::coasting_energy_loss(150.0, 0.0), 0.0);

    {
        double vi = 150.0;
        double ke_vi = physics::kinetic_energy(vi);
        double loss_far = physics::coasting_energy_loss(vi, 100000.0);

        check_true("coasting_energy_loss never exceeds kinetic_energy(vi)", loss_far <= ke_vi + 1e-6);
        check("coasting_energy_loss approaches kinetic_energy(vi) over a long distance", loss_far, ke_vi);
    }

    // -- time_to_reach_velocity -------------------------------------------
    check("time_to_reach_velocity(v, v, power) is ~0", physics::time_to_reach_velocity(150.0, 150.0, 300.0), 0.0, 1e-6);

    {
        double t_to_150 = physics::time_to_reach_velocity(150.0, 100.0, 300.0);
        double t_to_250 = physics::time_to_reach_velocity(250.0, 100.0, 300.0);
        check_true("time_to_reach_velocity(150,100,300) is positive", t_to_150 > 0.0);
        check_true("time_to_reach_velocity increases with a higher target speed", t_to_250 > t_to_150);
    }

    std::cout << "\n" << passed_checks << "/" << total_checks << " checks passed\n";
    return (passed_checks == total_checks) ? 0 : 1;
}
