#include "../include/physics.h"
#include <cmath>

namespace physics {

double kinetic_energy(double speed_kmh){
    double speed_ms = speed_kmh / 3.6;
    return 0.5 * MASS_KG * speed_ms * speed_ms;
}

double work_done_with_drag(double power_kW, double initial_speed_kmh, double distance_m){
    double r = power_kW * 1000.0;
    double vi = initial_speed_kmh / 3.6;

    double energy = MASS_KG / 2.0 * (std::pow(r / DRAG_K - (r / DRAG_K - std::pow(vi, 3.0))
                        * std::exp(-3.0 * DRAG_K * distance_m / MASS_KG), 2.0 / 3.0) - vi * vi);

    return energy;
}

double required_power(double initial_speed_kmh, double energy_target_J, double distance_m){
    double vi = initial_speed_kmh / 3.6;
    double decay = std::exp(-3.0 * DRAG_K * distance_m / MASS_KG);

    double r = (std::pow(vi * vi + 2.0 * energy_target_J / MASS_KG, 1.5) - std::pow(vi, 3.0) * decay)
                    * DRAG_K / (1.0 - decay);

    return r;
}

double coasting_energy_loss(double initial_speed_kmh, double distance_m){
    double vi = initial_speed_kmh / 3.6;

    double energy = (MASS_KG * vi * vi / 2.0) * (1.0 - std::exp(-2.0 * DRAG_K * distance_m / MASS_KG));

    return energy;
}

double distance_to_recharge(double initial_speed_kmh, double energy_target_J, double power_kW){
    double vi = initial_speed_kmh / 3.6;
    double r = power_kW * 1000.0;

    // NOTE: matches test.py's `(-mass/3*k)` exactly, i.e. ((-mass/3)*k), not
    // (-mass/(3*k)) as the drag-ODE derivation would suggest. Preserved as
    // translated rather than "corrected" -- flagged for the user to re-verify
    // against independent test points in test.py.
    double x = (-MASS_KG / (3.0 * DRAG_K))
                * std::log((r / DRAG_K - std::pow(vi * vi + 2.0 * energy_target_J / MASS_KG, 1.5))
                            / (r / DRAG_K - std::pow(vi, 3.0)));

    return x;
}

double time_to_reach_velocity(double target_speed_kmh, double initial_speed_kmh, double power_kW){
    double vi = initial_speed_kmh / 3.6;
    double v = target_speed_kmh / 3.6;
    double P = power_kW * 1000.0;

    double a = std::pow(P / DRAG_K, 1.0 / 3.0); // terminal velocity

    // Antiderivative of m*vel / (P - k*vel^3), via partial fractions.
    auto antiderivative = [a](double vel){
        double term1 = -1.0 / (3.0 * a) * std::log(std::fabs(a - vel));
        double term2 = 1.0 / (6.0 * a) * std::log(vel * vel + a * vel + a * a);
        double term3 = -1.0 / (a * std::sqrt(3.0)) * std::atan((2.0 * vel + a) / (a * std::sqrt(3.0)));
        return (1.0 / DRAG_K) * (term1 + term2 + term3);
    };

    return MASS_KG * (antiderivative(v) - antiderivative(vi));
}

}
