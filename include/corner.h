#pragma once    
#include "segment.h"
#include <vector>
#include <string>
#include "track-generation.h"

class Corner : public Segment{
    private:
        double entry_speed = 0.0;
        double exit_speed = 0.0;
        double time = 0.0;
        double energy = 0.0;
        std::vector<SpeedTraceType> speed_trace;

    public:
        Corner(std::string name, size_t start_index, size_t end_index);

        double get_entry_speed() const;
        double get_exit_speed() const;
        double get_time() const;
        double get_energy() const;
        std::vector<SpeedTraceType> get_speed_trace() const;

        // Parses necessary information from QSS csv. Independent from
        // Straight::parse_data() -- each derived class reads the csv
        // differently, they just happen to both fill in Segment::length.
        void parse_data(const std::vector<TrackDataPoint>& track_data, const std::vector<double>& curvature, const std::vector<double>& qss);
};