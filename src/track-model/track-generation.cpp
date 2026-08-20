#include "../../include/track-generation.h"
#include "../../include/physics.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cmath>
#include <limits>
#include <iostream>

namespace p = physics;

namespace track_gen{

    // Reads a track's raw X/Y racing-line data (no distance column) and computes
    // cumulative distance via chord-length summation between consecutive points.
    // Good approximation here since this data source (TUMFTM) is finely and
    // evenly spaced, unlike the noisier FastF1-derived telemetry used previously.
    // Input: file_path, the file location within /data/track-data/ folder
    // Output: A vector of TrackDataPoint, with distance_m computed rather than read
    const std::vector<TrackDataPoint> read_csv(const std::string& file_path){
        std::ifstream file(TRACK_CSV_FOLDER + file_path);
        if(!file.is_open()){
            throw std::runtime_error("Could not open track data file: " + file_path);
        }

        std::vector<double> x_positions;
        std::vector<double> y_positions;
        std::string line;

        std::getline(file, line);   // discard header row

        while(std::getline(file, line)){
            std::istringstream line_stream(line);
            std::string field;

            std::getline(line_stream, field, ',');
            double x_pos = std::stod(field);

            std::getline(line_stream, field, ',');
            double y_pos = std::stod(field);

            x_positions.push_back(x_pos);
            y_positions.push_back(y_pos);
        }

        std::vector<TrackDataPoint> points;
        points.resize(x_positions.size());
        points[0] = TrackDataPoint{0.0, x_positions[0], y_positions[0]};

        for(size_t i = 1; i < x_positions.size(); i++){
            double dx = x_positions[i] - x_positions[i - 1];
            double dy = y_positions[i] - y_positions[i - 1];
            double chord_dist = std::sqrt(dx*dx + dy*dy);
            points[i] = TrackDataPoint{points[i - 1].distance_m + chord_dist, x_positions[i], y_positions[i]};
        }

        return points;
    }

    // Computes the curvature of each data point of the track
    // Need to feed on the results of read_csv()
    // Input: track_data, A vector of TrackDataPoint
    // Output: A vector of curvature, corresponding to each data point of the track
    const std::vector<double> compute_curvature(const std::vector<TrackDataPoint>& track_data){
        std::vector<double> curvature;
        curvature.resize(track_data.size());

        double dis1 = 0.0;
        double dis2 = 0.0;
        const double wrap_around_dist = std::sqrt((track_data[track_data.size() - 1].x_pos - track_data[0].x_pos)*
                                                  (track_data[track_data.size() - 1].x_pos - track_data[0].x_pos) +
                                                  (track_data[track_data.size() - 1].y_pos - track_data[0].y_pos)*
                                                  (track_data[track_data.size() - 1].y_pos - track_data[0].y_pos));

        for(size_t i = 0; i < track_data.size(); i++){
            TrackDataPoint prev;
            TrackDataPoint next;

            if(i == 0){
                prev = track_data[track_data.size() - 1];
                next = track_data[1];
                dis1 = wrap_around_dist;
                dis2 = next.distance_m - track_data[i].distance_m;
            }
            else if(i == track_data.size() - 1){
                prev = track_data[track_data.size() - 2];
                next = track_data[0];
                dis1 = track_data[i].distance_m - prev.distance_m;
                dis2 = wrap_around_dist;
            }
            else{
                prev = track_data[i - 1];
                next = track_data[i + 1];
                dis1 = track_data[i].distance_m - prev.distance_m;
                dis2 = next.distance_m - track_data[i].distance_m;
            }

            double dx = (dis1*dis1*next.x_pos - dis2*dis2*prev.x_pos +
                        (dis2*dis2 - dis1*dis1)*track_data[i].x_pos) / (dis1*dis2*(dis1+dis2));
            double dy = (dis1*dis1*next.y_pos - dis2*dis2*prev.y_pos +
                        (dis2*dis2 - dis1*dis1)*track_data[i].y_pos) / (dis1*dis2*(dis1+dis2));

            double d2x = 2*(dis1*next.x_pos + dis2*prev.x_pos -
                         (dis2+dis1)*track_data[i].x_pos) / (dis1*dis2*(dis1+dis2));
            double d2y = 2*(dis1*next.y_pos + dis2*prev.y_pos -
                         (dis2+dis1)*track_data[i].y_pos) / (dis1*dis2*(dis1+dis2));

            double dot = dx*d2x + dy*d2y;
            double mag = std::sqrt(dx*dx + dy*dy);
            double x_numerator = (d2x - (dot/(mag*mag)) * dx)*(d2x - (dot/(mag*mag)) * dx);
            double y_numerator = (d2y - (dot/(mag*mag)) * dy)*(d2y - (dot/(mag*mag)) * dy);

            double k = std::sqrt(x_numerator + y_numerator)/ (mag*mag);

            curvature[i] = k;
        }

        return curvature;
    }

    // Computes the maximum velocities of each data point of the track, in m/s
    // Need to feed on the results of compute_curvature()
    // Input: curvature, A vector of curvature values of the track
    // Output: A vector of maximum speed at each curvature
    const std::vector<double> compute_vmax(const std::vector<double>& curvature){
        const double bound = p::FRICTION_COEFF * p::downforce_coeff(false, 10.0) / p::MASS_KG;
        std::vector<double> vmax;
        double max_speed = 0.0;

        vmax.resize(curvature.size());

        for(size_t i = 0; i < curvature.size(); i++){
            double k = curvature[i];

            if(k > bound){
                max_speed = std::sqrt(p::FRICTION_COEFF * p::GRAVITY / (k - bound));
            }
            else{
                max_speed = std::numeric_limits<double>::infinity();
            }

            // Cap it at 400km/h
            if(max_speed > 400.0 / 3.6) max_speed = 400.0 / 3.6;

            vmax[i] = max_speed;
        }
        return vmax;
    }

    // Runs the QSS forward/backward speed-profile simulation for the track.
    // Reads from read_csv() (the TUMFTM Silverstone.csv data source) with no
    // smoothing applied -- compute_curvature() runs directly on the raw data.
    const std::vector<double> qss(const std::string& file_name){
        // Hard coding this value for now because there are no other options
        const std::vector<TrackDataPoint> track_data = read_csv("Silverstone.csv");
        const std::vector<double> curvature = compute_curvature(track_data);
        const std::vector<double> vmax = compute_vmax(curvature);

        std::vector<double> output;
        std::vector<double> forward_pass;
        std::vector<double> backward_pass;
        output.resize(vmax.size());
        forward_pass.resize(vmax.size());
        backward_pass.resize(vmax.size());

        // Minimum speed on the track
        size_t min_index = std::min_element(vmax.begin(), vmax.end()) - vmax.begin();
        size_t forward_index = min_index;
        size_t backward_index = min_index;

        double forward_v = vmax[min_index];
        double backward_v = vmax[min_index];

        forward_pass[min_index] = forward_v;
        backward_pass[min_index] = backward_v;

        // Constants that don't change within the loop, therefore precomputed
        const double max_acc_pt1 = 0.5 * p::FRICTION_COEFF * p::GRAVITY; // Times by 0.5 because only the rear tires produces grip for acceleration
        const double max_acc_pt2 = 0.5 * p::FRICTION_COEFF * p::downforce_coeff(false, 10.0) - p::drag_coeff(false);
        const double max_decel_pt2 = p::FRICTION_COEFF * p::downforce_coeff(false, 10.0) + p::drag_coeff(false);
        const double max_lat_pt = p::FRICTION_COEFF * p::GRAVITY;
        const double engine_power = (p::ICE + p::MGU_K / 2) * 1000; // Using half of the MGU-K, a good middle ground

        // Because the distance at the end of the lap goes from a large value to 0,
        // I need to calculate it using positional data instead
        const double wrap_around_dist = std::sqrt((track_data[vmax.size() - 1].x_pos - track_data[0].x_pos)*
                                                  (track_data[vmax.size() - 1].x_pos - track_data[0].x_pos) +
                                                  (track_data[vmax.size() - 1].y_pos - track_data[0].y_pos)*
                                                  (track_data[vmax.size() - 1].y_pos - track_data[0].y_pos));

        double ds_forward = 0;
        double ds_backward = 0;

        // Forward and Backward Integerations
        for(size_t i = 0; i < vmax.size(); i++){
            // Forward pass ==========================================================
            double forward_v2 = forward_v * forward_v;
            double lat_forward = forward_v2 * curvature[forward_index];

            // Reaching the end of the lap
            if(forward_index == vmax.size() - 1){
                ds_forward = wrap_around_dist;
                forward_index = 0;
            }
            else{
                ds_forward = track_data[forward_index + 1].distance_m - track_data[forward_index].distance_m;
                forward_index += 1;
            }

            double max_acc = max_acc_pt1 + max_acc_pt2 * forward_v2 / p::MASS_KG;
            double max_lat_forward = max_lat_pt + p::FRICTION_COEFF * p::downforce(forward_v * 3.6, false, 10.0) / p::MASS_KG;
            double acc = max_acc * std::sqrt(std::max(0.0, 1.0 - (lat_forward / max_lat_forward)*(lat_forward / max_lat_forward)));
            double max_engine_acc = engine_power / (p::MASS_KG * forward_v) - p::drag(forward_v * 3.6, false) / p::MASS_KG;
            double min_acc = std::min(acc, max_engine_acc);

            forward_v = std::min(std::max(0.0, std::sqrt(forward_v2 + 2 * min_acc *  ds_forward)), vmax[forward_index]);
            forward_pass[forward_index] = forward_v;

            // Backward pass =========================================================
            double backward_v2 = backward_v * backward_v;
            double lat_backward = backward_v2 * curvature[backward_index];

            // Reaching the end of the lap
            if(backward_index == 0){
                ds_backward = wrap_around_dist;
                backward_index = vmax.size() - 1;
            }
            else{
                ds_backward = track_data[backward_index].distance_m - track_data[backward_index - 1].distance_m;
                backward_index -= 1;
            }

            double max_decel = 2.0 * max_acc_pt1 + max_decel_pt2 * backward_v2 / p::MASS_KG;
            double max_lat_backward = max_lat_pt + p::FRICTION_COEFF * p::downforce(backward_v * 3.6, false, 10.0) / p::MASS_KG;
            double decel = max_decel * std::sqrt(std::max(0.0, 1.0 - (lat_backward / max_lat_backward)*(lat_backward / max_lat_backward)));

            backward_v = std::min(std::max(0.0, std::sqrt(backward_v2 + 2 * decel * ds_backward)), vmax[backward_index]);
            backward_pass[backward_index] = backward_v;
        }

        // Merge forward and backward passes
        for(size_t i = 0; i < vmax.size(); i++){
            output[i] = std::min(forward_pass[i], backward_pass[i]);
        }

        return output;
    }

    // Writes the calculated speed, curvature etc into a file
    // Inputs: file_name, the file location to run QSS simulation on
    void write_csv(const std::string& file_name){
        auto results = qss("");
        const std::vector<TrackDataPoint> track_data = read_csv("Silverstone.csv");
        const std::vector<double> curvature = compute_curvature(track_data);
        const std::vector<double> vmax = compute_vmax(curvature);

        std::ofstream qss_file;
        qss_file.open(TRACK_CSV_FOLDER + "qss_silverstone_new.csv");

        if(!qss_file.is_open()){
            throw std::runtime_error("Could not open track data file when writing");
        }

        qss_file << "QSS Speed,Distance,Curvature,Max Velocity" << '\n';
        for(size_t i = 0; i < track_data.size(); i++){
            qss_file << results[i] * 3.6 << ',';
            qss_file << track_data[i].distance_m << ',';
            qss_file << curvature[i] << ',';
            qss_file << vmax[i] * 3.6 << '\n';
        }

        qss_file.close();
    }


    // ================================================================================================
    // Smoothing data using Cyclic cubic spline. With Thomas and Shermon-Morrison algorithms
    // Not used right now but good to be saved for the future

    // Solves a plain (non-cyclic) tridiagonal system a[i]*x[i-1] + b[i]*x[i] + c[i]*x[i+1] = d[i]
    // via the Thomas algorithm. a[0] and c[n-1] are unused (boundary rows have no left/right neighbor).
    static std::vector<double> thomas_solve(std::vector<double> a, std::vector<double> b,
                                             std::vector<double> c, std::vector<double> d){
        size_t n = b.size();
        for(size_t i = 1; i < n; i++){
            double m = a[i] / b[i - 1];
            b[i] -= m * c[i - 1];
            d[i] -= m * d[i - 1];
        }

        std::vector<double> x(n);
        x[n - 1] = d[n - 1] / b[n - 1];
        for(size_t i = n - 1; i-- > 0; ){
            x[i] = (d[i] - c[i] * x[i + 1]) / b[i];
        }
        return x;
    }

    // Solves a cyclic tridiagonal system: same as thomas_solve, but with two extra
    // "corner" entries -- alpha at A[n-1][0] and beta at A[0][n-1] -- from wrapping
    // around a closed loop. Uses the standard Sherman-Morrison correction (Numerical
    // Recipes' "cyclic" method): treat the corners as a rank-1 update to a plain
    // tridiagonal system, solve twice with thomas_solve, then combine.
    static std::vector<double> cyclic_tridiagonal_solve(std::vector<double> a, std::vector<double> b,
                                                         std::vector<double> c, std::vector<double> d,
                                                         double alpha, double beta){
        size_t n = b.size();
        double gamma = -b[0];   // arbitrary nonzero pivot, keeps the corrected diagonal well-scaled

        std::vector<double> b_corrected = b;
        b_corrected[0] -= gamma;
        b_corrected[n - 1] -= alpha * beta / gamma;

        std::vector<double> u(n, 0.0);
        u[0] = gamma;
        u[n - 1] = alpha;

        std::vector<double> x = thomas_solve(a, b_corrected, c, d);
        std::vector<double> z = thomas_solve(a, b_corrected, c, u);

        double fact = (x[0] + beta * x[n - 1] / gamma) / (1.0 + z[0] + beta * z[n - 1] / gamma);

        std::vector<double> result(n);
        for(size_t i = 0; i < n; i++){
            result[i] = x[i] - fact * z[i];
        }
        return result;
    }

    // Fits a periodic (closed-loop) natural cubic spline through (s[i], y[i]) pairs
    // and returns the second derivative S''(s[i]) at every knot. Everything else the
    // spline can give us -- S(s), S'(s) -- is recoverable from these plus the
    // original points, so this is the only system that actually needs solving.
    // Inputs: s, the arc-length/distance coordinate of each point (sorted)
    //         y, the value being interpolated (x_pos or y_pos)
    //         wrap_h, the real distance from the last point back around to the first
    // Output: second derivative of the fitted spline at each knot
    static std::vector<double> fit_periodic_spline_second_derivatives(const std::vector<double>& s,
                                                                       const std::vector<double>& y,
                                                                       double wrap_h){
        size_t n = s.size();
        std::vector<double> h(n);
        for(size_t i = 0; i + 1 < n; i++){
            h[i] = s[i + 1] - s[i];
        }
        h[n - 1] = wrap_h;

        std::vector<double> a(n), b(n), c(n), d(n);
        for(size_t i = 0; i < n; i++){
            size_t prev = (i == 0) ? n - 1 : i - 1;
            size_t next = (i + 1) % n;
            double h_prev = h[prev];
            double h_next = h[i];

            a[i] = h_prev;
            b[i] = 2.0 * (h_prev + h_next);
            c[i] = h_next;
            d[i] = 6.0 * ((y[next] - y[i]) / h_next - (y[i] - y[prev]) / h_prev);
        }

        // The two corner entries (row 0's coefficient of M[n-1], and row n-1's
        // coefficient of M[0]) both equal the wraparound interval width here --
        // pull them out of a/c and hand them to the cyclic solver separately.
        a[0] = 0.0;
        c[n - 1] = 0.0;
        return cyclic_tridiagonal_solve(a, b, c, d, wrap_h, wrap_h);
    }

    // First derivative of the fitted spline at knot i, evaluated from the interval
    // starting at i -- equal to the value from the interval ending at i, since the
    // spline is built to be C1-continuous at every knot by construction.
    static double spline_first_derivative(const std::vector<double>& s, const std::vector<double>& y,
                                          const std::vector<double>& M, double wrap_h, size_t i){
        size_t n = s.size();
        size_t next = (i + 1) % n;
        double h_i = (i + 1 < n) ? s[next] - s[i] : wrap_h;
        return (y[next] - y[i]) / h_i - h_i * (2.0 * M[i] + M[next]) / 6.0;
    }

    // Computes the curvature of each data point of the track using a periodic cubic
    // spline's exact analytical derivatives, instead of finite differences.
    // A separate function from compute_curvature() on purpose -- keeps the existing,
    // working finite-difference path available as a fallback. Not currently used by
    // qss() since the TUMFTM data source doesn't need it (see PROJECT_SPEC.md), but
    // kept available in case a noisier data source is used again in the future.
    // Input: track_data, A vector of TrackDataPoint (raw, unsmoothed)
    // Output: A vector of curvature, corresponding to each data point of the track
    const std::vector<double> compute_curvature_spline(const std::vector<TrackDataPoint>& track_data){
        size_t n = track_data.size();
        std::vector<double> curvature(n);

        std::vector<double> s(n), x(n), y(n);
        for(size_t i = 0; i < n; i++){
            s[i] = track_data[i].distance_m;
            x[i] = track_data[i].x_pos;
            y[i] = track_data[i].y_pos;
        }

        const double wrap_h = std::sqrt((track_data[n - 1].x_pos - track_data[0].x_pos) * (track_data[n - 1].x_pos - track_data[0].x_pos) +
                                        (track_data[n - 1].y_pos - track_data[0].y_pos) * (track_data[n - 1].y_pos - track_data[0].y_pos));

        const std::vector<double> Mx = fit_periodic_spline_second_derivatives(s, x, wrap_h);
        const std::vector<double> My = fit_periodic_spline_second_derivatives(s, y, wrap_h);

        for(size_t i = 0; i < n; i++){
            double dx = spline_first_derivative(s, x, Mx, wrap_h, i);
            double dy = spline_first_derivative(s, y, My, wrap_h, i);
            double d2x = Mx[i];
            double d2y = My[i];

            double dot = dx*d2x + dy*d2y;
            double mag = std::sqrt(dx*dx + dy*dy);
            double x_numerator = (d2x - (dot/(mag*mag)) * dx)*(d2x - (dot/(mag*mag)) * dx);
            double y_numerator = (d2y - (dot/(mag*mag)) * dy)*(d2y - (dot/(mag*mag)) * dy);

            curvature[i] = std::sqrt(x_numerator + y_numerator) / (mag*mag);
        }

        return curvature;
    }
}
