#pragma once
#include "fast-corner.h"
#include "slow-corner.h"
#include "straight.h"
#include <memory>
#include <vector>

class Track{

    private:
        std::vector<std::unique_ptr<Segment>> track;
        int index = 0;

    public:

        Track();                    // Initiallizes the track and builds the Silverstone circuit
        Segment* begin();           // The beginning segment of the track
        Segment* end();             // The ending segment of the track
        Segment* next(int index);   // The next segment of the track
        Segment* next();
        Segment* prev(int index);   // The previous segment of the track
        Segment* prev(); 
        Segment* at(int index);     // Given the index, what is the segment
        void incre();               // Move to the next segment
        void decre();               // Move to previous next segment
        void reset();               // Reset to the first segment
        int size();



};