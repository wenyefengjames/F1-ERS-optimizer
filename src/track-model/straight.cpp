#include "../../include/straight.h"

Straight::Straight(std::string name, double length) 
                : Segment(std::move(name), "Straight", length){}

Straight::Straight(std::string name, double length, bool sm, double sm_start, double sm_end) 
                 : Segment(std::move(name), "Straight", length, sm), sm_start(sm_start),
                     sm_end(sm_end){}

double Straight::get_sm_start() const{
    return sm_start;
}

double Straight::get_sm_end() const{
    return sm_end;
}