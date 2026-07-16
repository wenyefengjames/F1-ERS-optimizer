#include "../../include/slow-corner.h"

SlowCorner::SlowCorner(std::string name, double length, double time, double apex_min_speed, 
                      double exit_speed,double entry_speed, double throttle_percentage) 
                      : Segment(std::move(name), "SlowCorner", length, time),
                        apex_min_speed(apex_min_speed), exit_speed(exit_speed), 
                        entry_speed(entry_speed),
                        throttle_percentage(throttle_percentage){
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
