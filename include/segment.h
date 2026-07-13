#pragma once
#include <string>

class Segment {
    // A class that should be inherited by straight, slow-corner, and fast-corner. 
    // This class gives a baseline of what every type of segment should include.

    private:
        const std::string name; // Name of the segment
        const std::string type; // The type of the segment, e.g. straight, corner, braking.
        const double time; // Time taken on going through this segment, in seconds, accuracy to ms
        const double length; // The length of this segment of the track, in km

    public:
        Segment(std::string name, std::string type, double length, double time);
        
        // Getters
        std::string get_name() const;
        std::string get_type() const;
        double get_length() const;
        double get_time() const;

        virtual ~Segment();
    
};
