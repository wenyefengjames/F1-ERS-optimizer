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

    std::vector<TrackDataPoint> read_csv(const std::string& file_path){
        std::ifstream file(TRACK_CSV_FOLDER + file_path);
        if(!file.is_open()){
            throw std::runtime_error("Could not open track data file: " + file_path);
        }

        std::vector<TrackDataPoint> points;
        std::string line;

        std::getline(file, line);   // discard header row

        while(std::getline(file, line)){
            std::istringstream line_stream(line);
            std::string field;

            std::getline(line_stream, field, ',');
            double distance_m = std::stod(field);

            std::getline(line_stream, field, ',');
            double x_pos = std::stod(field);

            std::getline(line_stream, field, ',');
            double y_pos = std::stod(field);

            points.push_back(TrackDataPoint{distance_m, x_pos, y_pos});
        }

        return points;
    }

    const std::vector<double> compute_curvature(const std::vector<TrackDataPoint>& track_data){
        std::vector<double> curvature;
        curvature.resize(track_data.size());

        // Remove this after testing
        // double distance = 0;
        // double sum_mag = 0;

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

            // distance += std::sqrt((track_data[i].x_pos - prev.x_pos)*(track_data[i].x_pos - prev.x_pos) + (track_data[i].y_pos - prev.y_pos)*(track_data[i].y_pos - prev.y_pos));
            // sum_mag += mag;
        
            // std::cout << "index: " << i << "\t";
            // std::cout << "distance: " << distance << "\t";
            // std::cout << "mag: " << mag << "\n";

            curvature[i] = k;
        }

        // std::cout << "average_mag: " << sum_mag / track_data.size() << "\t";
        // std::cout << "distance factor: " << distance / 5823.474 << "\n";

        return curvature;
    }

    // Produces a vector of velocities in m/s
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

    const std::vector<double> qss(const std::string& file_name){
        // Hard coding this value for now because there are no other options
        const std::vector<TrackDataPoint> track_data = read_csv("silverstone_antonelli_quali.csv");
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

        // std::cout << "Min speed: " << forward_v * 3.6 << '\t'; 
        // std::cout << "Min index: " << min_index << '\n'; 

        forward_pass[min_index] = forward_v;
        backward_pass[min_index] = backward_v;

        // Constants that don't change within the loop, therefore precomputed
        const double max_acc_pt1 = 0.5 * p::FRICTION_COEFF * p::GRAVITY; // Times by 0.5 because only the rear tires produces grip for acceleration
        const double max_acc_pt2 = 0.5 * p::FRICTION_COEFF * p::downforce_coeff(false, 10.0) - p::drag_coeff(false);
        const double max_decel_pt2 = p::FRICTION_COEFF * p::downforce_coeff(false, 10.0) + p::drag_coeff(false);
        const double max_lat_pt = p::FRICTION_COEFF * p::GRAVITY;

        // std::cout << "max_acc_pt1: " << max_acc_pt1 << '\t'; 
        // std::cout << "max_acc_pt2: " << max_acc_pt2 << '\t'; 
        // std::cout << "max_decel_pt2: " << max_decel_pt2 << '\t'; 
        // std::cout << "max_lat_pt: " << max_lat_pt << '\n'; 

        // Because the distance at the end of the lap goes from 5800 to 0, 
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
            double max_engine_acc = p::ICE * 1000 / (p::MASS_KG * forward_v) - p::drag(forward_v * 3.6, false) / p::MASS_KG;
            double min_acc = std::min(acc, max_engine_acc);

            forward_v = std::min(std::max(0.0, std::sqrt(forward_v2 + 2 * min_acc *  ds_forward)), vmax[forward_index]);
            forward_pass[forward_index] = forward_v;

            // std::cout << "Forward index: " << forward_index << '\t';
            // std::cout << "lat_forward: " << lat_forward << '\t';
            // std::cout << "forward_v: " << forward_v << '\t';
            // std::cout << "max_acc: " << max_acc << '\t';
            // std::cout << "max_lat_forward: " << max_lat_forward << '\t';
            // std::cout << "acc: " << acc << '\t';
            // std::cout << "max_engine_acc: " << max_engine_acc << '\t';
            // std::cout << "forward curavture: " << curvature[forward_index] << '\t';
            // std::cout << "min_acc: " << min_acc << '\n';

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

            // std::cout << "backward_index: " << backward_index << '\t';
            // std::cout << "backward_kmh: " << backward_v * 3.6 << '\t';
            // std::cout << "backward_v: " << backward_v << '\t';
            // std::cout << "max_decel: " << max_decel << '\t';
            // std::cout << "max_lat_backward: " << max_lat_backward << '\t';
            // std::cout << "decel: " << decel << '\t';
            // std::cout << "backward curavture: " << curvature[backward_index] << '\t';
            // std::cout << "decel: " << decel << '\n';
        }

        // Merge forward and backward passes
        for(size_t i = 0; i < vmax.size(); i++){
            output[i] = std::min(forward_pass[i], backward_pass[i]);
        }

        return output;
    }

    void write_csv(){
        auto results = qss("");

        std::ofstream qss_file;
        qss_file.open(TRACK_CSV_FOLDER + "qss_antonelli.csv");

        if(!qss_file.is_open()){
            throw std::runtime_error("Could not open track data file when writing");
        }

        qss_file << "Speed" << '\n';
        for(const auto& i : results){
            qss_file << i * 3.6 << '\n';
        }

        qss_file.close();

    }
}