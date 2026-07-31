#pragma once
#include <string>
#include "segment.h"

class FastCorner : public Segment{
    // This class represents fast corners

    private:
        const double apex_min_speed;        // The minimum speed of the car at the center of apex
        const double exit_speed;            // The exit speed of the car out of the corner
        const double throttle_percentage;   // Limitation, a constant for now
        const double entry_to_apex_length;
        const double apex_to_exit_length;

    public:
        FastCorner(std::string name, double entry_to_apex_length, double apex_to_exit_length,
                   double apex_min_speed, double exit_speed, double throttle_percentage);
        
        // Getters
        double get_apex_min_speed() const;
        double get_exit_speed() const;
        double get_throttle_percentage() const;
        double get_entry_to_apex_length() const;
        double get_apex_to_exit_length() const;
};