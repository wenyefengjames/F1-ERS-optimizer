#pragma once
#include <string>
#include <vector>

struct TrackDataPoint{
    double distance_m;
    double x_pos;
    double y_pos;
};

namespace track_gen{
    inline const std::string TRACK_CSV_FOLDER = "data/track-data/";

    // TO DO
    const std::vector<double> qss(const std::string& file_name);
    void write_csv();

    // Testing purpose only, shouldn't exist here
    std::vector<TrackDataPoint> read_csv(const std::string& file_path);
    const std::vector<double> compute_curvature(const std::vector<TrackDataPoint>& track_data);
    const std::vector<double> compute_vmax(const std::vector<double>& curvature);
}