#include "../../include/corner.h"
#include "../../include/physics.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace tg = track_gen;
namespace p = physics;

Corner::Corner(std::string name, size_t start_index, size_t end_index) :
               Segment(std::move(name), SegmentType::Corner, start_index, end_index){}

double Corner::get_entry_speed() const{
    return this->entry_speed;
}

double Corner::get_exit_speed() const{
    return this->exit_speed;
}

double Corner::get_time() const{
    return this->time;
}

double Corner::get_energy() const{
    return this->energy;
}

std::vector<SpeedTraceType> Corner::get_speed_trace() const{
    return this->speed_trace;
}

// TODO: read qss_silverstone.csv over [get_start_index(), get_end_index()] and set
// entry_speed/exit_speed/this->length/speed_trace (and eventually time/energy, per
// our earlier discussion -- left for you to design). Stub for now.
void Corner::parse_data(const std::vector<TrackDataPoint>& track_data, const std::vector<double>& curvature, const std::vector<double>& qss){
    size_t start_index = get_start_index();
    size_t end_index = get_end_index();

    this->entry_speed = qss[start_index]*3.6;
    this->exit_speed = qss[end_index+1]*3.6;

    std::vector<SpeedTraceType> trace;
    double time = 0;
    double energy = 0;

    for(size_t i = start_index; i <= end_index; i++){
        double mean_speed = (qss[i] + qss[i+1]) / 2.0;
        double mean_curvature = (curvature[i] + curvature[i+1]) / 2.0;

        double delta =  2 * (track_data[i+1].distance_m - track_data[i].distance_m) / (qss[i] + qss[i+1]);
        double engine_power_ratio = (p::max_acc_tyres(mean_speed*3.6, mean_curvature, 10.0, false) + 
                                    (p::drag(mean_speed*3.6, false) / p::MASS_KG)) / 
                                    (p::max_acc_engine(mean_speed*3.6, p::ICE, false) + 
                                    (p::drag(mean_speed*3.6, false) / p::MASS_KG));
        double recharge_rate = p::ICE * (1 - std::clamp(engine_power_ratio, 0.0, 1.0));
        double energy_increase = recharge_rate*1000 * delta; 

        SpeedTraceType temp = {.speed_kmh = qss[i] * 3.6, .distance_m = track_data[i].distance_m};
        trace.push_back(temp);

        time += delta;
        energy += energy_increase;
    }

    this->time = time;
    this->energy = energy;
    this->speed_trace = trace;
    this->length = track_data[end_index + 1].distance_m - track_data[start_index].distance_m;
}

