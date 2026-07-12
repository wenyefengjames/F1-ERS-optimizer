#include "../../include/straight.h"

Straight::Straight(std::string name, double length, double time) 
                      : Segment(std::move(name), "Straight", length, time){}