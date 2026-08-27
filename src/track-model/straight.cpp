#include "../../include/straight.h"

Straight::Straight(std::string name, size_t start_index, size_t end_index) :
                   Segment(std::move(name), SegmentType::Straight, start_index, end_index){}

Straight::Straight(std::string name, size_t start_index, size_t end_index, double sm_start, double sm_end) :
                   Segment(std::move(name), SegmentType::Straight, start_index, end_index, true),
                   sm_start(sm_start), sm_end(sm_end){}

double Straight::get_sm_start() const{
    return sm_start;
}

double Straight::get_sm_end() const{
    return sm_end;
}

// TODO: read qss_silverstone.csv over [get_start_index(), get_end_index()] and set
// this->length from the Distance column. Left as a stub -- filling this in yourself.
void Straight::parse_data(const std::vector<TrackDataPoint>& track_data, const std::vector<double>& curvature){
    size_t start_index = get_start_index();
    size_t end_index = get_end_index();

    // If end_index is the last index
    if(end_index == track_data.size() - 1){
        length = track_data[end_index].distance_m - track_data[start_index].distance_m + 5;
    }
    else{
        length = track_data[end_index + 1].distance_m - track_data[start_index].distance_m;
    }
}