#include "../../include/track.h"

// Initiallizes the track and builds the Silverstone circuit
Track::Track(){

    // TO DO, add straight mode to relavent sections of the track
    // track.push_back(std::make_unique<Straight>("Hamilton Straight             ", 410, true, 25, 325));
    track.push_back(std::make_unique<Straight>("Hamilton Straight             ", 452, true, 25, 325));
    // track.push_back(std::make_unique<Straight>("Hamilton Straight             ", 410));
    track.push_back(std::make_unique<FastCorner>("Abbey, Turn 1                 ", 101, 106, 307, 295, 100));
    track.push_back(std::make_unique<FastCorner>("Farm, Turn 2                  ", 82, 200, 286, 263, 100));

    track.push_back(std::make_unique<SlowCorner>("Turn 3                      ", 75, 76, 5.04, 263, 108, 141, 50));
    track.push_back(std::make_unique<SlowCorner>("Turn 4                      ", 73, 112, 5.06, 141, 87, 184, 50));
    track.push_back(std::make_unique<FastCorner>("Turn 5                      ", 83, 44, 237, 257, 100));

    // track.push_back(std::make_unique<Straight>("Wellington Straight           ", 585, true, 0, 585));
    track.push_back(std::make_unique<Straight>("Wellington Straight           ", 595, true, 0, 595));

    track.push_back(std::make_unique<SlowCorner>("Turn 6                      ", 135, 88, 4.13, 280, 167, 184, 43));
    track.push_back(std::make_unique<SlowCorner>("Turn 7                      ", 74, 145, 6.17, 284, 114, 230, 50));

    // track.push_back(std::make_unique<Straight>("Woodcote Turn 8 - Old Pit Straight", 690, true, 0, 590));
    track.push_back(std::make_unique<Straight>("Woodcote Turn 8 - Old Pit Straight", 667, true, 0, 567));
    // track.push_back(std::make_unique<Straight>("Woodcote Turn 8 - Old Pit Straight", 690));
    // track.push_back(std::make_unique<Straight>("Woodcote, Turn 8                 ", 319, true, 0, 319));
    // track.push_back(std::make_unique<Straight>("Old Pit Straight", 371, true, 0, 271));

    track.push_back(std::make_unique<FastCorner>("Copse, Turn 9                 ", 109, 56, 284, 287, 100));

    track.push_back(std::make_unique<Straight>("Turn 9 - Turn 10 Straight ", 384.5));

    track.push_back(std::make_unique<FastCorner>("Maggots, Turn 10              ", 75.5, 46, 285, 0, 100));
    track.push_back(std::make_unique<FastCorner>("Maggots, Turn 11              ", 45, 93, 278, 270, 100));
    track.push_back(std::make_unique<FastCorner>("Becketts, Turn 12             ", 77, 54, 259, 0, 80));
    track.push_back(std::make_unique<FastCorner>("Becketts, Turn 13             ", 112, 68.5, 219, 244, 55));

    // track.push_back(std::make_unique<Straight>("Turn 14 - Hanger Straight     ", 813.5, true, 90.5, 813.5));
    track.push_back(std::make_unique<Straight>("Turn 14 - Hanger Straight     ", 785, true, 90.5, 785));
    // track.push_back(std::make_unique<Straight>("Turn 14  ", 90.5));
    // track.push_back(std::make_unique<Straight>("Hanger Straight  ", 723, true, 0, 723));

    track.push_back(std::make_unique<SlowCorner>("Stowe, Turn 15                ", 132.5, 959, 3.38, 290, 236, 249, 64));

    track.push_back(std::make_unique<Straight>("Vale Straight                 ", 290.5));
    // track.push_back(std::make_unique<Straight>("T15 - Pit Entry Straight  ", 157.5));
    // track.push_back(std::make_unique<Straight>("Pit Entry - T16 Straight  ", 133));

    track.push_back(std::make_unique<SlowCorner>("Turn 16                     ", 65, 30.5, 3.16, 260, 105, 113, 40));
    track.push_back(std::make_unique<FastCorner>("Turn 17                     ", 36.5, 63, 130, 184, 40));
    track.push_back(std::make_unique<FastCorner>("Turn 18                     ", 113, 51, 226, 243, 100));
}

// The current segment that the index is pointing to
Segment* Track::current(){
    return track.at(index).get();
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

// Return the current index
int Track::get_index(){
    return index;
}


