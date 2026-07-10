#pragma once
#include <string>

class Segment {
    // A class that contains all necessary attributes of a segment of a track.

    private:
        std::string name; // Name of the segment
        std::string type; // The type of the segment, e.g. straight, corner, braking.
        double free_harvest_energy; // How much you can harvest on this segment, doesn't impact delta, in MJ
        double extra_harvest_energy; // How much extra you can harvest on this segment, impacts delta, in MJ
        double deploy_energy; // How much you can deploy on this segment, in MJ
        double time; // Time taken on going through this segment, in seconds, accuracy to ms
        double distance; // The length of this segment of the track, in km

    public:
        Segment(std::string name, int free_harvest_energy, int extra_harvest_energy, 
                int deploy_energy, double time);
        
        // Getters ----------------------------------------------------------
        std::string get_name() const;
        int get_extra_harvest_energy() const;
        int get_free_harvest_energy() const;
        int get_deploy_energy() const;
        double get_time() const;
            
        // Setters ----------------------------------------------------------
        void set_name(std::string name);
        void set_extra_harvest_energy(int extra_harvest_energy);
        void set_free_harvest_energy(int free_harvest_energy);
        void set_deploy_energy(int deploy_energy);
        void set_time(double time);
};
