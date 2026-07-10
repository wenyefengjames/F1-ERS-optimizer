#pragma once
#include <string>
#include "segment.h"

class Straight : public Segment{
    // The class to represent straights

    public:
        Straight(std::string name, double length, double time);
        
};