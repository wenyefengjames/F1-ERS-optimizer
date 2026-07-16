#pragma once
#include <string>
#include "segment.h"

class SlowCorner : public Segment{
    // This class represents slow corners

    private:
        const double apex_min_speed; // The minimum speed of the car at the center of apex
        const double exit_speed; // The exit speed of the car out of the corner
        const double throttle_percentage; // Limitation, a constant for now
        const double entry_speed;

    public:
        SlowCorner(std::string name, double length, double time, double apex_min_speed, 
                    double exit_speed, double entry_speed, double throttle_percentage);
        
        // Getters
        double get_apex_min_speed() const;
        double get_exit_speed() const;
        double get_throttle_percentage() const;
        double get_entry_speed() const;
    
};