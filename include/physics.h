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
    inline constexpr double BRAKING_DECEL = 5.5 * 9.8;     // m/s^-2, assuming that the braking force is 5.5g

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

    // The curve of how the deployment rate decreases
    // Assuming the power output decreases linearly with respect to speed
    // Inputs: speed_kmh - current speed, in km/h
    //         mom - if we have manual overide mode, boolean
    // Output: the power output, in kW
    double taper_curve(double speed_kmh, double mom);

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
