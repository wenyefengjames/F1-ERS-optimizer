#include "../../include/segment.h"

Segment::Segment(std::string name, SegmentType type, size_t start_index, size_t end_index)
                : name(std::move(name)), type(type), start_index(start_index), end_index(end_index){}

Segment::Segment(std::string name, SegmentType type, size_t start_index, size_t end_index, bool sm)
                : name(std::move(name)), type(type), start_index(start_index), end_index(end_index), sm(sm){}

const std::string& Segment::get_name() const{
    return this->name;
}

SegmentType Segment::get_type() const{
    return this->type;
}

bool Segment::get_sm() const{
    return this->sm;
}

size_t Segment::get_start_index() const{
    return this->start_index;
}

size_t Segment::get_end_index() const{
    return this->end_index;
}

double Segment::get_length() const{
    return this->length;
}

Segment::~Segment() = default;


