#include "../../include/segment.h"

Segment::Segment(std::string name, SegmentType type, double length)
                : name(std::move(name)), type(type), length(length){
}

Segment::Segment(std::string name, SegmentType type, double length, bool sm)
                : name(std::move(name)), type(type), length(length), sm(sm){
}

const std::string& Segment::get_name() const{
    return this->name;
}

SegmentType Segment::get_type() const{
    return this->type;
}

double Segment::get_length() const{
    return this->length;
}

bool Segment::get_sm() const{
    return this->sm;
}

Segment::~Segment() = default;


