#include "../../include/slow-corner.h"

SlowCorner::SlowCorner(std::string name, double entry_to_apex_length, double apex_to_exit_length,
                       double time, double apex_min_speed, double exit_speed, 
                       double entry_speed, double throttle_percentage) 
                      : Segment(std::move(name), "SlowCorner", entry_to_apex_length + apex_to_exit_length),
                        entry_to_apex_length(entry_to_apex_length), apex_to_exit_length(apex_to_exit_length),  
                        time(time), apex_min_speed(apex_min_speed), exit_speed(exit_speed), 
                        entry_speed(entry_speed), throttle_percentage(throttle_percentage){
}

double SlowCorner::get_apex_min_speed() const{
    return this->apex_min_speed;
}

double SlowCorner::get_exit_speed() const{
    return this->exit_speed;
}

double SlowCorner::get_throttle_percentage() const{
    return this->throttle_percentage;
}

double SlowCorner::get_entry_speed() const{
    return this->entry_speed;
}

double SlowCorner::get_time() const{
    return this->time;
}

double SlowCorner::get_entry_to_apex_length() const{
    return this->entry_to_apex_length;
}
double SlowCorner::get_apex_to_exit_length() const{
    return this->apex_to_exit_length;    
}
