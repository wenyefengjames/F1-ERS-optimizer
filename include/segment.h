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
        const size_t start_index;       // Start of the segment in QSS CSV file
        const size_t end_index;         // End of the segment in QSS CSV file
        const bool sm = false;          // Whether straight mode is on or not

    protected:
        double length = 0.0;            // Distance covered by the segment, set by each derived
                                         // class's own parse_data() -- protected so they can write to it

    public:
        Segment(std::string name, SegmentType type, size_t start_index, size_t end_index);
        Segment(std::string name, SegmentType type, size_t start_index, size_t end_index, bool sm);

        // Getters
        const std::string& get_name() const;
        SegmentType get_type() const;
        bool get_sm() const;
        size_t get_start_index() const;
        size_t get_end_index() const;
        double get_length() const;

        virtual ~Segment();
    
};
