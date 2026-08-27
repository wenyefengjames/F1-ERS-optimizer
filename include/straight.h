#pragma once
#include <string>
#include "segment.h"
#include "track-generation.h"

class Straight : public Segment{
    // The class to represent straights
    private:
        double sm_start = -1;
        double sm_end = -1;


    public:
        Straight(std::string name, size_t start_index, size_t end_index);
        Straight(std::string name, size_t start_index, size_t end_index, double sm_start, double sm_end);

        double get_sm_start() const;
        double get_sm_end() const;

        // Parses necessary information (currently just length) from the QSS csv.
        // Independent from Corner::parse_data() -- each derived class reads the
        // csv differently, they just happen to both fill in Segment::length.
        void parse_data(const std::vector<TrackDataPoint>& track_data, const std::vector<double>& curvature);
};