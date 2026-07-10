#include "segment.h"

Segment::Segment(std::string name, int free_harvest_energy, int extra_harvest_energy, 
                int deploy_energy, double time, ){
    this->name = name;
    this->free_harvest_energy = free_harvest_energy;
    this->extra_harvest_energy = extra_harvest_energy;
    this->deploy_energy = deploy_energy;
    this->time = time;
    this->distance = distance;
    this->type = type;
}
        
// Getters ----------------------------------------------------------
std::string Segment::get_name() const{
    return this->name;
}
int Segment::get_extra_harvest_energy() const{
    return this->extra_harvest_energy;
}

int Segment::get_free_harvest_energy() const{
    return this->free_harvest_energy;
}
int Segment::get_deploy_energy() const{
    return this->deploy_energy;
}
double Segment::get_time() const{
    return this->time;
}
        
// Setters ----------------------------------------------------------
void Segment::set_name(std::string name){
    this->name = name;
}
void Segment::set_extra_harvest_energy(int extra_harvest_energy){
    this->extra_harvest_energy = extra_harvest_energy;
}
void Segment::set_free_harvest_energy(int free_harvest_energy){
    this->free_harvest_energy = free_harvest_energy;
}
void Segment::set_deploy_energy(int deploy_energy){
    this->deploy_energy = deploy_energy;
}
void Segment::set_time(double time){
    this->time = time;
}



