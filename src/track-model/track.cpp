#include "../../include/track.h"

// Initiallizes the track and builds the Silverstone circuit
Track::Track(){
    track.push_back(std::make_unique<Straight>("Hamilton Straight          ", 458, 0.0));
    // track.push_back(std::make_unique<FastCorner>("Abbey, Turn 1", 167, 0.0, 300, 300, 100));
    // track.push_back(std::make_unique<FastCorner>("Farm, Turn 2", 262, 0.0, 300, 300, 100));
    track.push_back(std::make_unique<FastCorner>("Abbey, Turn 1              ", 167, 0.0, 300, 290, 100));
    track.push_back(std::make_unique<FastCorner>("Farm, Turn 2               ", 262, 0.0, 280, 275, 100));
    track.push_back(std::make_unique<SlowCorner>("Turn 3 - 5                 ", 0.0, 12.3, 87, 240, 263, 50));
    track.push_back(std::make_unique<Straight>("Wellington Straight        ", 595, 0.0));
    track.push_back(std::make_unique<SlowCorner>("Turn 6 - 8                 ", 0.0, 10.37, 114, 230, 273, 43));
    track.push_back(std::make_unique<Straight>("Woodcote - Old Pit Straight", 667, 0.0));
    track.push_back(std::make_unique<FastCorner>("Copse, Turn 9              ", 119, 0.0, 282, 287, 100));
    track.push_back(std::make_unique<Straight>("Turn 9 - Turn 10           ", 476, 0.0));
    track.push_back(std::make_unique<FastCorner>("Maggots, Turn 11           ", 167, 0.0, 278, 271, 100));
    track.push_back(std::make_unique<FastCorner>("Becketts, Turn 12          ", 150, 2.01, 252, 242, 80));
    track.push_back(std::make_unique<FastCorner>("Becketts, Turn 13          ", 200, 3.09, 230, 244, 55));
    track.push_back(std::make_unique<Straight>("Turn 14 - Hanger Straight  ", 785, 0.0));
    track.push_back(std::make_unique<SlowCorner>("Stowe, Turn 15             ", 0.0, 3.37, 236, 249, 290, 64));
    track.push_back(std::make_unique<Straight>("Vale straight              ", 286, 0.0));
    track.push_back(std::make_unique<SlowCorner>("Turn 16 - 18               ", 0.0, 9.24, 102, 241, 254, 40));
}

// The beginning segment of the track
Segment* Track::begin(){
    return track.at(0).get();
}

// The ending segment of the track
Segment* Track::end(){
    return track.at(track.size() - 1).get();
}

// The next segment of the track
Segment* Track::next(int i){
    return track.at((i + 1) % track.size()).get();
}


Segment* Track::next(){
    return track.at((index + 1) % track.size()).get();
}

// The previous segment of the track
Segment* Track::prev(int i){
    return track.at((i + track.size() - 1) % track.size()).get();
}

Segment* Track::prev(){
    return track.at((index + track.size() - 1) % track.size()).get();
}

Segment*  Track::at(int index){
    return track.at(index).get();
}

// Move to the next segment
void Track::incre(){
    index = (index + 1) % track.size();
}

// Move to previous next segment
void Track::decre(){
    index = (index + track.size() - 1) % track.size();
}

// Reset to the first segment
void Track::reset(){
    index = 0;
}

// Return the number of segments in the track
int Track::size(){
    return track.size();
}


