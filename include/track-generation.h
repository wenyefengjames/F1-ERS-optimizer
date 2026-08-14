#pragma once
#include <string>
#include <vector>

struct TrackDataPoint{
    double distance_m;
    double x_pos;
    double y_pos;
};

namespace track_gen{
    inline constexpr std::string TRACK_CSV_FOLDER = "data/track-data/";

    // TO DO
    const std::vector<double> qss(const std::string& file_name);
}