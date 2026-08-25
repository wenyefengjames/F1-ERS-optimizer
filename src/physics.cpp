#include "../include/physics.h"
#include <iostream>
#include <numbers>

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
        if(initial_speed_kmh < 0){ // Input validity check
            return -1;
        }

        double speed_ms = initial_speed_kmh / 3.6;
        double init_energy = 0.5 * speed_ms*speed_ms*MASS_KG;

        if(init_energy + energy_J < 0) return -1;

        return std::sqrt(2 * (init_energy + energy_J) / MASS_KG) * 3.6;
    }

    double work_done_with_drag(double power_kW, double initial_speed_kmh, double distance_m, bool sm_on){
        if(power_kW > ICE + MGU_K || power_kW < 0 || initial_speed_kmh < 0 || distance_m < 0) {
            return -1;
        }

        double r = power_kW * 1000.0;
        double vi = initial_speed_kmh / 3.6;
        double energy = MASS_KG / 2.0 * (std::pow(r / drag_coeff(sm_on) - (r / drag_coeff(sm_on) - std::pow(vi, 3.0))
                            * std::exp(-3.0 * drag_coeff(sm_on) * distance_m / MASS_KG), 2.0 / 3.0) - vi * vi);

        return energy;
    }

    double required_power(double initial_speed_kmh, double energy_target_J, double distance_m, bool mom, bool sm_on){
        if(initial_speed_kmh < 0 || distance_m < 0) {
            return -1;
        }

        double vi = initial_speed_kmh / 3.6;
        double decay = std::exp(-3.0 * drag_coeff(sm_on) * distance_m / MASS_KG);
        double r = (std::pow(vi * vi + 2.0 * energy_target_J / MASS_KG, 1.5) - std::pow(vi, 3.0) * decay)
                        * drag_coeff(sm_on) / (1.0 - decay);

        // This means that the power required isn't achievable 
        if(r / 1000 > taper_curve(initial_speed_kmh, mom) + ICE){
            return -1;
        }

        return r;
    }

    double distance_to_recharge(double initial_speed_kmh, double energy_target_J, double power_kW, bool sm_on){
        if(initial_speed_kmh < 0 || energy_target_J < 0 || power_kW < 0) {
            return -1;
        }

        double vi = initial_speed_kmh / 3.6;
        double r = power_kW * 1000.0;

        double x = (-MASS_KG / (3.0 * drag_coeff(sm_on)))
                    * std::log((r / drag_coeff(sm_on) - std::pow(vi * vi + 2.0 * energy_target_J / MASS_KG, 1.5))
                                / (r / drag_coeff(sm_on) - std::pow(vi, 3.0)));

        return x;
    }

    std::optional<TimeEnergyResult> time_to_reach_speed_over_distance(double initial_speed_kmh, double final_speed_kmh, double distance_m, bool mom, bool sm_on){
        if(initial_speed_kmh <= 0 || final_speed_kmh <= 0 || distance_m <= 0){
            return std::nullopt;
        }

        // Superclip
        if(initial_speed_kmh > final_speed_kmh){
            double energy_diff = kinetic_energy(final_speed_kmh) - kinetic_energy(initial_speed_kmh);
            double power = required_power(initial_speed_kmh, energy_diff, distance_m, mom, sm_on);

            // power < 0 indicates that it needs breaking
            if(power < 0) return std::nullopt;

            double time = time_to_reach_velocity(final_speed_kmh, initial_speed_kmh, power / 1000, sm_on);
            double energy_harvested = (ICE * 1000 - power) * time;

            return TimeEnergyResult{.energy_J = energy_harvested, .time_s = time};
        }
        else if(initial_speed_kmh == final_speed_kmh){
            double time = distance_m / (initial_speed_kmh / 3.6);

            // If positive number for 'energy' it indicates harvest, negative indicates deployment
            double energy = time * (ICE * 1000 - required_power(initial_speed_kmh, 0, distance_m, mom, sm_on));  

            return TimeEnergyResult{.energy_J = energy, .time_s = time};
        }
        else{
            double current_kmh = initial_speed_kmh; 
            double total_deployed_distance = 0.0;
            double total_energy_deployed = 0.0;
            double total_time = 0.0;

            double MGU_K_step_size = 10; //kW
            double deploy_power = 0; // kW

            bool flag = true;

            // A loop to try to find the minimal amount of power from MGU-K needed to reach the target speed
            while(flag && deploy_power <= MGU_K){
                current_kmh = initial_speed_kmh; 
                total_deployed_distance = 0.0;
                total_energy_deployed = 0.0;
                total_time = 0.0;

                // std::cout << "Step size: ==============" << deploy_power << "\n";

                // Numerical method to approximate net KE gain
                while(total_deployed_distance < distance_m && current_kmh < final_speed_kmh){
                    double current_power = std::min(taper_curve(current_kmh, mom), deploy_power);
                    double ke_gained = work_done_with_drag(current_power + ICE, current_kmh, current_kmh * DELTA_T / 3.6, sm_on);
                    total_deployed_distance += current_kmh * DELTA_T / 3.6;
                    current_kmh = reverse_ke(current_kmh, ke_gained);
                    total_energy_deployed += current_power * DELTA_T * 1000;
                    total_time += DELTA_T;

                    // if((int)(total_time * 100) % 20 == 0){
                    //     std::cout << "Power: " << current_power << "\t";
                    //     std::cout << "current_kmh: " << current_kmh << "\t";
                    //     std::cout << "total_energy_deployed: " << total_energy_deployed << "\t";
                    //     std::cout << "total_time: " << total_time << "\n";
                    // }
                    

                    // Check if the current speed has hit the tapering limit
                    if((current_kmh >= 290 && !mom) || (current_kmh >= 337 && mom)) {
                        double remaining_distance = distance_m - total_deployed_distance;

                        auto query = [](const TaperedDeploymentResult& field){return field.speed_kmh;};
                        auto result = search_taper_table(mom, sm_on, final_speed_kmh, query);

                        if(result == std::nullopt) continue;

                        // If the necessary distance to reach the speed is longer than the remaining distance, then its unreachable
                        if(result->distance_m > remaining_distance) return std::nullopt;

                        // After adding these values the loop should exit
                        total_time += result->time_s;
                        total_deployed_distance += result->distance_m;
                        total_energy_deployed += result->energy_J;
                        current_kmh = result->speed_kmh;
                    }
                }
                
                // Speed reachable
                if(total_deployed_distance <= distance_m && current_kmh >= final_speed_kmh) {
                    flag = false;
                    break;
                }

                deploy_power += MGU_K_step_size;
            }
            
            // Speed not reachable
            if(flag) return std::nullopt;

            // Calculate the rest of the distance
            double extra_time = (distance_m - total_deployed_distance) / (final_speed_kmh / 3.6);
            double energy = extra_time * (ICE * 1000 - required_power(final_speed_kmh, 0, distance_m, mom, sm_on));
            
            total_time += extra_time;
            return TimeEnergyResult{.energy_J = -total_energy_deployed + energy, .time_s = total_time};
        }
    }

    double time_to_reach_velocity(double target_speed_kmh, double initial_speed_kmh, double power_kW, bool sm_on){
        if(target_speed_kmh <= 0 || initial_speed_kmh <= 0 || power_kW < 0) return -1;
        
        double vi = initial_speed_kmh / 3.6;
        double v = target_speed_kmh / 3.6;
        double P = power_kW * 1000.0;

        double a = std::pow(P / drag_coeff(sm_on), 1.0 / 3.0); // terminal velocity

        // Antiderivative of m*vel / (P - k*vel^3), via partial fractions.
        auto antiderivative = [a](double vel, bool sm_on){
            double term1 = -1.0 / (3.0 * a) * std::log(std::fabs(a - vel));
            double term2 = 1.0 / (6.0 * a) * std::log(vel * vel + a * vel + a * a);
            double term3 = -1.0 / (a * std::numbers::sqrt3) * std::atan((2.0 * vel + a) / (a * std::numbers::sqrt3));
            return (1.0 / drag_coeff(sm_on)) * (term1 + term2 + term3);
        };

        return MASS_KG * (antiderivative(v, sm_on) - antiderivative(vi, sm_on));
    }
    
    double taper_curve(double speed_kmh, bool mom){
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

        return drag_coeff(sm_on) * speed_kmh * speed_kmh;
    }
    
    double drag_coeff(bool sm_on){
        static const double with_sm = 0.5 * AIR_DENSITY * FRONTAL_AREA * SM_DRAG_COEFF;
        static const double without_sm = 0.5 * AIR_DENSITY * FRONTAL_AREA * CM_DRAG_COEFF;
        if(sm_on) return with_sm;
        else return without_sm;
    }

    // The current implementation doesn't take into the account of any parameter apart from speed
    // TO DO, Will be improved in the future
    double downforce(double speed_kmh, bool sm_on, double delta){
        if(speed_kmh < 0) return -1;

        speed_kmh = speed_kmh / 3.6;

        return downforce_coeff(sm_on, delta) * speed_kmh * speed_kmh;
    }

    double downforce_coeff(bool sm_on, double delta){
        if(delta < 0) return -1;

        static const double with_sm = 0.5 * AIR_DENSITY * FRONTAL_AREA * SM_DOWNFORCE_COEFF;
        static const double without_sm = 0.5 * AIR_DENSITY * FRONTAL_AREA * CM_DOWNFORCE_COEFF;
        if(sm_on) return with_sm;
        else return without_sm;
    }

    // Overloaded function where SM is true 
    TaperedDeploymentResult energy_deployed_with_taper(double initial_kmh, double distance, double sm_start, double sm_end, bool mom){
        double current_kmh = initial_kmh; 
        double total_deployed_distance = 0.0;
        double total_energy_deployed = 0.0;
        double total_time = 0.0;
        double ke_gained = 0;
        
        // Numerical method to approximate net KE gain
        while(total_deployed_distance < distance){
            double current_power = taper_curve(current_kmh, mom);

            // sm_start being less than 0 indicates that SM is false
            if(sm_start >=0 && total_deployed_distance >= sm_start && total_deployed_distance <= sm_end){
                ke_gained = work_done_with_drag(current_power + ICE, current_kmh, current_kmh * DELTA_T / 3.6, true);
            }
            else{
                ke_gained = work_done_with_drag(current_power + ICE, current_kmh, current_kmh * DELTA_T / 3.6, false);
            }

            total_deployed_distance += current_kmh * DELTA_T / 3.6;
            current_kmh = reverse_ke(current_kmh, ke_gained);
            total_energy_deployed += current_power * DELTA_T * 1000;
            total_time += DELTA_T;

            // Check if the current speed has hit the tapering limit
            if((current_kmh >= 290 && !mom) || (current_kmh >= 337 && mom)) {
                double remaining_distance = distance - total_deployed_distance;

                auto query = [](const TaperedDeploymentResult& field){return field.distance_m;};
                std::optional<TaperedDeploymentResult> result;

                if(sm_start >=0 && total_deployed_distance >= sm_start && total_deployed_distance <= sm_end){
                    result = search_taper_table(mom, true, remaining_distance, query);
                }
                else{
                    result = search_taper_table(mom, false, remaining_distance, query);
                }
                if(result == std::nullopt) continue;

                // After adding these values the loop should exit
                total_time += result->time_s;
                total_deployed_distance += result->distance_m;
                total_energy_deployed += result->energy_J;
                current_kmh = result->speed_kmh;
            }
        }

        TaperedDeploymentResult output = {.speed_kmh = current_kmh, .energy_J = total_energy_deployed,
                                          .time_s = total_time, .distance_m = total_deployed_distance};
        return output; 
    }

    // Private function. This builds the required lookup table for tapering
    std::vector<TaperedDeploymentResult> build_taper_table(bool mom, bool sm_on){

        std::vector<TaperedDeploymentResult> output;
        double current_kmh = 0.0; 
        double total_deployed_distance = 0.0;
        double total_energy_deployed = 0.0;
        double total_time = 0.0;
        const double delta_t = 0.02; 
        const double max_time = 4;

        if(mom)current_kmh = 337; 
        else   current_kmh = 290; 
        
        // This line basically gives table.begin() a meaning. If the value tried to search within
        // the table is less than the minimum value, searching will give the iterator pointing to 
        // table.begin(). This line makes sure that it is genuinely not in the table, rather than just
        // the needed value is small so it wants the first field of the table.
        output.push_back({.speed_kmh = current_kmh, .energy_J = total_energy_deployed,
                          .time_s = total_time, .distance_m = total_deployed_distance});

        // Generate the table
        while(total_time <= max_time && current_kmh <= 355){
            double current_power = taper_curve(current_kmh, mom);
            double ke_gained = work_done_with_drag(current_power + ICE, current_kmh, current_kmh * delta_t / 3.6, sm_on);
            total_deployed_distance += current_kmh * delta_t / 3.6;
            current_kmh = reverse_ke(current_kmh, ke_gained);
            total_energy_deployed += current_power * delta_t * 1000;
            total_time += delta_t;

            output.push_back({.speed_kmh = current_kmh, .energy_J = total_energy_deployed,
                              .time_s = total_time, .distance_m = total_deployed_distance});
        }

        return output;
    }

    // Private function. This outputs the required lookup table for tapering
    // Constructs the lookup tables once, then cache them and returns the cached table
    const std::optional<std::vector<TaperedDeploymentResult>>& taper_table(bool mom, bool sm_on){
        static const std::optional<std::vector<TaperedDeploymentResult>> no_mom_sm_table = build_taper_table(false, true);
        static const std::optional<std::vector<TaperedDeploymentResult>> no_mom_no_sm_table = build_taper_table(false, false);
        static const std::optional<std::vector<TaperedDeploymentResult>> mom_sm_table = build_taper_table(true, true);
        static const std::optional<std::vector<TaperedDeploymentResult>> no_table = std::nullopt;

        if(mom){
            if(sm_on) return mom_sm_table;
            else return no_table;   // mom = true and sm_on = false won't have a table. 
                                    // It is very unlikely that the car will hit the taper entry speed
        } 
        else{
            if(sm_on) return no_mom_sm_table;
            else return no_mom_no_sm_table;
        }
    }

    // Calculate maximum braking deceleration possible
    double max_deceleration(double current_speed_kmh, double curvature, double delta, bool sm){
        // Precomputed values because they don't depend on the parameters
        static const double max_decel_pre1 = 0.5 * FRICTION_COEFF * GRAVITY; // Times by 0.5 because only the rear tires produces grip for accelerati
        static const double max_decel_pre2 = FRICTION_COEFF * downforce_coeff(sm, delta); // Tyre grip only -- drag is handled separately below
        static const double max_lat_pre = FRICTION_COEFF * GRAVITY;

        double current_speed_ms = current_speed_kmh / 3.6;

        double lateral_acc = current_speed_ms*current_speed_ms * curvature;

        // Tyre-only grip, each axis capped independently so the friction circle they feed
        // into stays a real circle instead of the downforce-scaled axis blowing past the tyre's
        // actual physical ceiling at high speed
        double max_decel_tyre = std::min(2.0 * max_decel_pre1 + max_decel_pre2 * current_speed_ms*current_speed_ms / MASS_KG, TYRE_FORCE_CAP);
        double max_lateral_acc = std::min(max_lat_pre + FRICTION_COEFF * downforce(current_speed_kmh, sm, delta) / MASS_KG, TYRE_FORCE_CAP);

        double decel_tyre = max_decel_tyre * std::sqrt(std::max(0.0, 1.0 - (lateral_acc / max_lateral_acc)*(lateral_acc / max_lateral_acc)));

        // Drag isn't a tyre force, so it stacks on top of the capped tyre grip uncapped
        double decel = decel_tyre + drag(current_speed_kmh, sm) / MASS_KG;

        return decel;
    }
    
    // Calculate maximum acceleration possible
    double max_acceleration(double current_speed_kmh, double curvature, double delta, double engine_power_kW, bool sm){
        double tyre_acc = max_acc_tyres(current_speed_kmh, curvature, delta, sm);

        double engine_acc = max_acc_engine(current_speed_kmh, engine_power_kW, sm);
        double min_acc = std::min(tyre_acc, engine_acc);

        return min_acc;
    }

    // Calculate maximum acceleration possible from tyres
    double max_acc_tyres(double current_speed_kmh, double curvature, double delta, bool sm){
        // Precomputed values because they don't depend on the parameters
        static const double max_acc_pre1 = 0.5 * FRICTION_COEFF * GRAVITY; // Times by 0.5 because only the rear tires produces grip for acceleration
        static const double max_acc_pre2 = 0.5 * FRICTION_COEFF * downforce_coeff(sm, delta) - drag_coeff(sm);
        static const double max_lat_pre = FRICTION_COEFF * GRAVITY;

        double current_speed_ms = current_speed_kmh / 3.6;
        double lateral_acc = current_speed_ms*current_speed_ms * curvature;

        double max_acc = max_acc_pre1 + max_acc_pre2 * current_speed_ms*current_speed_ms / MASS_KG;
        double max_lateral_acc = max_lat_pre + FRICTION_COEFF * downforce(current_speed_kmh, sm, delta) / MASS_KG;
        double acc = max_acc * std::sqrt(std::max(0.0, 1.0 - (lateral_acc / max_lateral_acc)*(lateral_acc / max_lateral_acc)));

        return acc;
    }

    // Calculate maximum acceleration through engine
    double max_acc_engine(double current_speed_kmh, double engine_power_kW, bool sm){
        if(current_speed_kmh < 0 || engine_power_kW < 0 || engine_power_kW > 750) return -1;

        double engine_acc = engine_power_kW * 1000 / (MASS_KG * current_speed_kmh / 3.6) - drag(current_speed_kmh, sm) / MASS_KG;

        return engine_acc;
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
