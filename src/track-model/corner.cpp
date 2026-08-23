#include "../../include/corner.h"

Corner::Corner(double entry_speed, double exit_speed, double time, double length, double energy,
               std::vector<SpeedTraceType> speed_trace, std::string name) : 
              entry_speed(entry_speed), exit_speed(exit_speed), time(time), energy(energy),
              speed_trace(std::move(speed_trace)), Segment(name, SegmentType::Corner, length){}

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

