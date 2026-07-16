#pragma once

// Drag-aware kinematics/energy formulas for the car, translated from test.py
// once their correctness was validated there. Stateless by design (free
// functions in a namespace, not a class) -- there's no object identity here,
// just math shared by Car and the optimizer.
namespace physics {
    inline constexpr double MASS_KG = 768.0;               // car mass, kg
    inline constexpr double DRAG_AREA_COEFF = 0.8;         // CdA, m^2
    inline constexpr double AIR_DENSITY = 1.225;           // rho, kg/m^3
    inline constexpr double DRAG_K = 0.5 * AIR_DENSITY * DRAG_AREA_COEFF;
    inline constexpr double BATTERY_CAPACITY = 4.0;        // MJ
    inline constexpr double MGU_K = 350;                   // kW
    inline constexpr double ICE = 400;                     // kW
    inline constexpr double BRAKING_DECEL = 5.5 * 9.8;     // m/s^-2

    // Kinetic energy at a given speed (no drag).
    // Input: speed, in km/h
    // Output: kinetic energy, in Joules
    double kinetic_energy(double speed_kmh);

    // Final speed after adding extra kinetic energy to the car
    // Input: speed, in km/h
    //        energy, in Joules 
    // Output: speed, in km/h
    double reverse_ke(double initial_speed_kmh, double energy);

    // Kinetic energy gained (excludes the car's initial KE) after travelling
    // distance_m at constant engine power, net of drag losses.
    // Inputs: power_kW - engine power, in kW
    //         initial_speed_kmh - initial speed, in km/h
    //         distance_m - distance travelled at this power, in meters
    // Output: kinetic energy gained, in Joules
    double work_done_with_drag(double power_kW, double initial_speed_kmh, double distance_m);

    // Inverse of work_done_with_drag: the constant engine power required to
    // gain energy_target_J of KE within distance_m, starting from initial_speed_kmh.
    // Inputs: initial_speed_kmh - initial speed, in km/h
    //         energy_target_J - kinetic energy that needs to be gained, in Joules
    //         distance_m - distance available to gain it, in meters
    // Output: required engine power, in Watts
    double required_power(double initial_speed_kmh, double energy_target_J, double distance_m);

    // Kinetic energy lost to drag while coasting (no throttle) over distance_m.
    // Inputs: initial_speed_kmh - initial speed, in km/h
    //         distance_m - distance travelled while coasting, in meters
    // Output: kinetic energy lost to drag, in Joules
    double coasting_energy_loss(double initial_speed_kmh, double distance_m);

    // Distance needed, at constant engine power_kW, to gain energy_target_J
    // of KE starting from initial_speed_kmh.
    // Inputs: initial_speed_kmh - initial speed, in km/h
    //         energy_target_J - kinetic energy that needs to be gained, in Joules
    //         power_kW - constant engine power used to gain it, in kW
    // Output: distance required, in meters
    double distance_to_recharge(double initial_speed_kmh, double energy_target_J, double power_kW);

    // Time to accelerate from initial_speed_kmh to target_speed_kmh under
    // constant engine power_kW, net of drag.
    // Inputs: target_speed_kmh - speed to reach, in km/h
    //         initial_speed_kmh - starting speed, in km/h
    //         power_kW - constant engine power, in kW
    // Output: time to reach target_speed_kmh, in seconds
    double time_to_reach_velocity(double target_speed_kmh, double initial_speed_kmh, double power_kW);


}
