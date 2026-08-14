#pragma once
#include <vector>
#include <optional>
#include <algorithm>
#include <cmath>
#include <iterator>

struct TaperedDeploymentResult{
    double speed_kmh = 0.0;
    double energy_J = 0.0;
    double time_s = 0.0;
    double distance_m = 0.0;
};

struct TimeEnergyResult {
    double energy_J = 0.0;
    double time_s = 0.0;
};



// Drag-aware kinematics/energy formulas for the car, translated from test.py
// once their correctness was validated there. Stateless by design (free
// functions in a namespace, not a class) -- there's no object identity here,
// just math shared by Car and the optimizer.
namespace physics {
    inline constexpr double MASS_KG = 768.0;                // car mass, kg
    inline constexpr double DRAG_AREA_COEFF = 0.8;          // CdA, m^2
    inline constexpr double AIR_DENSITY = 1.225;            // rho, kg/m^3
    inline constexpr double FRONTAL_AREA = 1.45;            // A, m^2
    inline constexpr double CM_DRAG_COEFF = 1.0;            // Cd, Corner mode, no unit
    inline constexpr double SM_DRAG_COEFF = 0.7;            // Cd, Stragiht mode, no unit
    inline constexpr double DOWNFORCE_COEFF = 1.9;          // Cd, Stragiht mode, no unit
    inline constexpr double BATTERY_CAPACITY = 4.0;         // MJ
    inline constexpr double MGU_K = 350;                    // kW
    inline constexpr double ICE = 400;                      // kW
    inline constexpr double BRAKING_DECEL = 5.5 * 9.8;      // m/s^-2, assuming that the braking force is 5.5g
    inline constexpr double DELTA_T = 0.01;                  // s, change in time for numerical approximation

    // Kinetic energy at a given speed (no drag).
    // Input: speed, in km/h
    // Output: kinetic energy, in Joules
    double kinetic_energy(double speed_kmh);

    // Turning kinetic energy into speed in kmh
    // Input: kinetic energy, in Joules
    // Output: speed, in km/h
    double ke_to_speed(double energy_J);

    // Final speed after adding extra kinetic energy to the car
    // Input: speed, in km/h
    //        energy, in Joules 
    // Output: speed, in km/h
    double reverse_ke(double initial_speed_kmh, double energy_J);

    // Kinetic energy gained (excludes the car's initial KE) after travelling
    // distance_m at constant engine power, net of drag losses.
    // Inputs: power_kW - engine power, in kW
    //         initial_speed_kmh - initial speed, in km/h
    //         distance_m - distance travelled at this power, in meters
    //         sm_on - indicates if the car has SM
    // Output: kinetic energy gained, in Joules
    double work_done_with_drag(double power_kW, double initial_speed_kmh, double distance_m, bool sm_on);

    // Inverse of work_done_with_drag: the constant engine power required to
    // gain energy_target_J of KE within distance_m, starting from initial_speed_kmh.
    // Inputs: initial_speed_kmh - initial speed, in km/h
    //         energy_target_J - kinetic energy that needs to be gained, in Joules
    //         distance_m - distance available to gain it, in meters
    //         mom - indicates if the car has MOM
    //         sm_on - indicates if the car has SM
    // Output: required engine power, in Watts
    double required_power(double initial_speed_kmh, double energy_target_J, double distance_m, bool mom, bool sm_on);

    // Distance needed, at constant engine power_kW, to gain energy_target_J
    // of KE starting from initial_speed_kmh.
    // Inputs: initial_speed_kmh - initial speed, in km/h
    //         energy_target_J - kinetic energy that needs to be gained, in Joules
    //         power_kW - constant engine power used to gain it, in kW
    //         sm_on - indicates if the car has SM
    // Output: distance required, in meters
    double distance_to_recharge(double initial_speed_kmh, double energy_target_J, double power_kW, bool sm_on);

    // Time to accelerate from initial_speed_kmh to target_speed_kmh under
    // constant engine power_kW, net of drag.
    // IMPORTANT: Use time_to_reach_speed_over_distance() function over this one. 
    // That one includes this function with better use cases
    // Inputs: target_speed_kmh - speed to reach, in km/h
    //         initial_speed_kmh - starting speed, in km/h
    //         power_kW - constant engine power, in kW
    //         sm_on- whether the car has SM
    // Output: time to reach target_speed_kmh, in seconds
    double time_to_reach_velocity(double target_speed_kmh, double initial_speed_kmh, double power_kW, bool sm_on);

    // Time taken to accelerate from initial_speed_kmh to final_speed_kmh
    // While covering distance_m
    // Inputs: initial_speed_kmh - speed to reach, in km/h
    //         final_speed_kmh - starting speed, in km/h
    //         distance_m - distance to cover, in meters
    //         mom - whether the car has MOM
    //         sm_on- whether the car has SM
    // Output: Time and energy harvested/deployed. If energy field is positive, it is harvesting.
    //         Otherwise it is deploying.
    std::optional<TimeEnergyResult> time_to_reach_speed_over_distance(double initial_speed_kmh, double final_speed_kmh, double distance_m, bool mom, bool sm_on);

    // The curve of how the deployment rate decreases
    // Assuming the power output decreases linearly with respect to speed
    // Inputs: speed_kmh - current speed, in km/h
    //         mom - if we have manual overide mode, boolean
    // Output: the power output, in kW
    double taper_curve(double speed_kmh, bool mom);

    // Calculates the drag on the car
    // Inputs: speed_kmh - current speed, in km/h
    //         sm_on - whether the straight mode is on or not
    // Output: drag force, in N
    double drag(double speed_kmh, bool sm_on);

    // Calculates the drag coefficient of the car, independent of speed
    // This is made a seperate function because in the future this can get more
    // complicated, e.g. trailing a car will make drag coeff lower
    // Inputs: sm_on - whether the car has straight mode or not
    // Output: the drag coefficient, no unit (there is but i cba it doesn't matter)
    double drag_coeff(bool sm_on);

    // Calculates the downforce on the car
    // Inputs: speed_kmh - current speed, in km/h
    //         sm_on - whether the straight mode is on or not
    //         delta - how far ahead is the car in front
    // Output: drag force, in N
    double downforce(double speed_kmh, bool sm_on, double delta);

    // Calculates the downforce coefficient of the car, independent of speed
    // Inputs: sm_on - whether the car has straight mode or not
    //         delta - how far ahead is the car in front
    // Output: the downforce coefficient, no unit (there is but i cba it doesn't matter)
    double downforce_coeff(bool sm_on, double delta);

    // For a given distance of deployment, estimate the amount of energy deployed
    // With consideration of tapering
    // Inputs: initial_kmh - initial speed, in km/h
    //         distance - how long to deploy energy for
    //         sm_start - where the SM starts. SET THIS TO -1 TO INDICATE NO SM
    //         sm_end - where the SM ends. SET THIS TO -1 TO INDICATE NO SM
    //         mom - indicates if the car has MOM
    // Output: energy gained with deployment, in Joules
    TaperedDeploymentResult energy_deployed_with_taper(double initial_kmh, double distance, double sm_start, double sm_end, bool mom);

    // TODO: Leaving this function here for now. Remove this line from this file after testing
    const std::optional<std::vector<TaperedDeploymentResult>>& taper_table(bool mom, bool sm_on);

    // Binary searches over taper tables. Returns std::nullopt if value not found
    // Inputs: mom - indicates if the car has MOM
    //         sm_on - indicates if the car has Straight line mode
    //         query_value - value to find. Can be speed, time, energy and distance
    //         get_field - lambda function that indicates what field the query_value belongs to
    // Output: If value within range. Returns the other attributes associated with the query value
    //         If value not in the table. Returns std::nullopt to indicate not in the table
    template<typename T>
    std::optional<TaperedDeploymentResult> search_taper_table(bool mom, bool sm_on,
                                                    double query_value, T get_field){

        auto comparitor = [get_field](const TaperedDeploymentResult& entry, double value)
            {
                return get_field(entry) < value;
            };

        const std::optional<std::vector<TaperedDeploymentResult>>& table = taper_table(mom, sm_on);
        if (table == std::nullopt) return std::nullopt;

        auto it = std::lower_bound(table->begin(), table->end(), query_value, comparitor);
        
        // Value doesn't exist in the table
        if(it == table->begin() || it == table->end()) return std::nullopt;

        auto lower = std::prev(it);
        double field_lower = get_field(*lower);
        double field_upper = get_field(*it);
        double t = (query_value - field_lower) / (field_upper - field_lower);

        return TaperedDeploymentResult{
            std::lerp(lower->speed_kmh, it->speed_kmh, t),
            std::lerp(lower->energy_J,  it->energy_J,  t),
            std::lerp(lower->time_s,    it->time_s,    t),
            std::lerp(lower->distance_m, it->distance_m, t)
        };
    };

    // Harvesting methods ==============================================================

    // Calculates the amount of energy harvested during braking
    // Inputs: current_speed_kmh - current speed, in km/h
    //         target_speed_kmh - target speed, in km/h
    // Output: Energy harvested during braking towards the target speed, in Joules
    double braking_harvest(double current_speed_kmh, double target_speed_kmh); 

    // Calculates the amount of energy harvested during coasting
    // Inputs: time - time spent during coast, in seconds
    // Output: Energy harvested during coasting, in Joules
    double coasting_harvest(double time);

    // Calculates the amount of energy harvested during superclipping
    // Inputs: clip_rate - the amount of engine output that goes to recharge, in kW
    //         time - time spent during superclipping, in seconds
    // Output: Energy harvested during superclipping, in Joules
    double superclipping(double clip_rate_kW, double time);

    // Calculates the amount of energy harvested during partial throttle
    // Inputs: throttle_percentage - percentage of throttle, between 0-100
    //         time - time spent during partial throttle, in seconds
    // Output: Energy harvested during partial throttle, in Joules
    double partial_throttle_harvest(double throttle_percentage, double time);
}
