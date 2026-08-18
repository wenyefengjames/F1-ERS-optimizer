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
    // Generate a QSS simulation for the given track
    // Inputs: file_name, the file location within /data/track-data/
    // Outputs: a vector of speed as a result of the simulation
    const std::vector<double> qss(const std::string& file_name);

    // Writes the calculated speed, curvature etc into a file
    // Inputs: file_name, the file location to run QSS simulation on
    void write_csv(const std::string& file_name);
}