#pragma once
#include <string>

enum class SegmentType {Straight, FastCorner, SlowCorner, Corner};

struct SpeedTraceType{
    double speed_kmh;
    double distance_m;
};

class Segment {
    // A class that should be inherited by straight, slow-corner, and fast-corner. 
    // This class gives a baseline of what every type of segment should include.

    private:
        const std::string name;         // Name of the segment
        const SegmentType type;         // The type of the segment
        const double length;            // The length of this segment of the track, in km
        const bool sm = false;          // Whether straight mode is on or not

    public:
        Segment(std::string name, SegmentType type, double length);
        Segment(std::string name, SegmentType type, double length, bool sm);
        
        // Getters
        const std::string& get_name() const;
        SegmentType get_type() const;
        double get_length() const;
        bool get_sm() const;

        virtual ~Segment();
    
};
