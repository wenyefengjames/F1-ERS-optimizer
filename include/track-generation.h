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

    const std::vector<TrackDataPoint> read_csv(const std::string& file_path);
    const std::vector<double> compute_curvature(const std::vector<TrackDataPoint>& track_data);
    const std::vector<double> compute_vmax(const std::vector<double>& curvature);

    // Generate a QSS simulation for the track (TUMFTM Silverstone.csv data source)
    // Inputs: file_name, the file location within /data/track-data/
    // Outputs: a vector of speed as a result of the simulation
    const std::vector<double> qss(const std::string& file_name);

    // Writes the calculated speed, curvature etc into a file
    // Inputs: file_name, the file location to run QSS simulation on
    void write_csv(const std::string& file_name);

    // Computes curvature via a periodic cubic spline's exact derivatives, instead
    // of the finite-difference approach in compute_curvature(). Not currently used
    // by qss() -- kept available in case a noisier data source needs it later.
    const std::vector<double> compute_curvature_spline(const std::vector<TrackDataPoint>& track_data);
}
