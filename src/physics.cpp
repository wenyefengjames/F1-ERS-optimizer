#include "../include/physics.h"
#include <cmath>
#include <algorithm>

namespace physics {

    double kinetic_energy(double speed_kmh){
        double speed_ms = speed_kmh / 3.6;
        return 0.5 * MASS_KG * speed_ms * speed_ms;
    }

    double ke_to_speed(double energy_J){
        if(energy_J < 0){ // Input validity check
            return -1;
        }

        return std::sqrt(2 * energy_J / MASS_KG) * 3.6;
    } 

    double reverse_ke(double initial_speed_kmh, double energy_J){
        double speed_ms = initial_speed_kmh / 3.6;
        return std::sqrt(speed_ms * speed_ms + 2 * energy_J / MASS_KG) * 3.6;
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
    
    double taper_curve(double speed_kmh, double mom){
        if(speed_kmh < 0) return -1;

        double power_output; // in kW

        if(mom){
            if(speed_kmh <= 337) power_output = 350;
            else power_output = 350 + (speed_kmh - 337)*-19.444444;
        }
        else{
            if(speed_kmh <= 290) power_output = 350; 
            else power_output = 350 + (speed_kmh - 290)*-5.3846154;
        }

        if(power_output < 0) power_output = 0;

        return power_output;
    }

    double drag(double speed_kmh, bool sm_on){
        if(speed_kmh < 0) return -1;

        speed_kmh = speed_kmh / 3.6;

        if(sm_on) {
            return 0.5 * AIR_DENSITY * FRONTAL_AREA * SM_DRAG_COEFF * speed_kmh * speed_kmh;
        } 
        
        return 0.5 * AIR_DENSITY * FRONTAL_AREA * CM_DRAG_COEFF * speed_kmh * speed_kmh;
    }

    TaperedDeploymentResult energy_deployed_with_taper(double initial_kmh, double distance, bool mom){
        double current_kmh = initial_kmh; 
        double total_deployed_distance = 0.0;
        double total_energy_deployed = 0.0;
        double total_time = 0.0;
        
        // Numerical method to approximate net KE gain
        while(total_deployed_distance < distance){
            double current_power = taper_curve(current_kmh, mom);
            double ke_gained = work_done_with_drag(current_power + ICE, current_kmh, current_kmh * DELTA_T / 3.6);
            current_kmh = reverse_ke(current_kmh, ke_gained);
            total_deployed_distance += current_kmh * DELTA_T / 3.6;
            total_energy_deployed += (current_power + ICE) * DELTA_T * 1000;
            total_time += DELTA_T;
        }

        TaperedDeploymentResult output = {current_kmh, total_energy_deployed, total_time, total_deployed_distance};

        return output; 
    }

    // Harvesting methods ==============================================================

    double braking_harvest(double current_speed_kmh, double target_speed_kmh){
        if(current_speed_kmh < 0 || target_speed_kmh < 0 || current_speed_kmh < target_speed_kmh) return -1;

        return (current_speed_kmh - target_speed_kmh)/(BRAKING_DECEL * 3.6) * MGU_K * 1000;
    }

    double coasting_harvest(double time){
        if(time < 0) return -1;

        return time * MGU_K * 1000;
    }

    double superclipping(double clip_rate_kW, double time){
        if(time < 0 || clip_rate_kW < 0) return -1;

        clip_rate_kW = std::min(MGU_K, clip_rate_kW);
        return time * clip_rate_kW * 1000;
    }

    double partial_throttle_harvest(double throttle_percentage, double time){
        if(throttle_percentage > 100 || throttle_percentage < 0 || time < 0) return -1;

        double recharge_rate = std::min(MGU_K, (100 - throttle_percentage) * 0.01 * ICE);
        double energy = time * recharge_rate * 1000;

        return energy;
    }

}
