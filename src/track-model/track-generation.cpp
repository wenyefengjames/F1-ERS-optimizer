#include "../../include/track-generation.h"
#include "../../include/physics.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cmath>
#include <limits>

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

    const std::vector<double> compute_curvature(const std::string& file_name){
        // Hard code it to silverstone for now, because that is the only track of concern
        std::vector<TrackDataPoint> track_data = read_csv("silverstone_antonelli_quali.csv");
        std::vector<double> curvature;
        curvature.reserve(track_data.size());

        for(size_t i = 0; i < track_data.size(); i++){
            TrackDataPoint prev;
            TrackDataPoint next;

            if(i == 0){
                prev = track_data[track_data.size() - 1];
                next = track_data[1];
            }
            else if(i == track_data.size() - 1){
                prev = track_data[track_data.size() - 2];
                next = track_data[0];
            }
            else{
                prev = track_data[i - 1];
                next = track_data[i + 1];                
            }
            double dis1 = track_data[i].distance_m - prev.distance_m;
            double dis2 = next.distance_m - track_data[i].distance_m;

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

    const std::vector<double> compute_vmax(const std::string& file_name){
        const std::vector<double> curvature = compute_curvature(file_name);
        const double bound = p::FRICTION_COEFF * p::downforce_coeff(false, 10.0) / p::MASS_KG;
        std::vector<double> vmax;
        double max_speed = 0.0;

        vmax.reserve(curvature.size());
        
        for(size_t i = 0; i < curvature.size(); i++){
            double k = curvature[i];

            if(k > bound){
                max_speed = std::sqrt(p::FRICTION_COEFF * p::GRAVITY / (k - bound)) * 3.6; // Translate m/s into km/h
            }
            else{
                max_speed = std::numeric_limits<double>::infinity();
            }

            vmax[i] = max_speed; 
        }
        return vmax;
    }

    const std::vector<double> qss(const std::string& file_name){
        const std::vector<double> vmax = compute_vmax(file_name);


    
    }
}