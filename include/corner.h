#pragma once    
#include "segment.h"
#include <vector>
#include <string>

class Corner : public Segment{
    private:
        const double entry_speed;
        const double exit_speed;
        const double time;
        const double energy;
        const std::vector<SpeedTraceType> speed_trace;

    public:
        Corner(double entry_speed, double exit_speed, double time, double length, double energy,
               std::vector<SpeedTraceType> speed_trace, std::string name);

        double get_entry_speed() const;
        double get_exit_speed() const;
        double get_time() const;
        double get_energy() const;
        std::vector<SpeedTraceType> get_speed_trace() const;
};