#include "fast-corner.h"

FastCorner::FastCorner(std::string name, double length, double time, double apex_min_speed, 
                      double exit_speed, double throttle_percentage) 
                      : Segment(std::move(name), "FastCorner", length, time),
                        apex_min_speed(apex_min_speed), exit_speed(exit_speed), 
                        throttle_percentage(throttle_percentage){
}

double FastCorner::get_apex_min_speed() const{
    return this->apex_min_speed;
}

double FastCorner::get_exit_speed() const{
    return this->exit_speed;
}

double FastCorner::get_throttle_percentage() const{
    return this->throttle_percentage;
}
