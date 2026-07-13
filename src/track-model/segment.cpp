#include "../../include/segment.h"

Segment::Segment(std::string name, std::string type, double length, double time)
                : name(std::move(name)), type(std::move(type)), time(time), length(length){
}

std::string Segment::get_name() const{
    return this->name;
}
std::string Segment::get_type() const{
    return this->type;
}
double Segment::get_length() const{
    return this->length;
}
double Segment::get_time() const{
    return this->time;
}

Segment::~Segment() = default;


