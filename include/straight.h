#pragma once
#include <string>
#include "segment.h"

class Straight : public Segment{
    // The class to represent straights
    private:
        double sm_start = -1;
        double sm_end = -1;

    public:
        Straight(std::string name, double length);
        Straight(std::string name, double length, bool sm, double sm_start,
                 double sm_end);
        
        double get_sm_start() const;
        double get_sm_end() const;
};