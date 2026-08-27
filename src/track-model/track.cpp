#include "../../include/track.h"

// Initiallizes the track and builds the Silverstone circuit
Track::Track(){

    add_straight("Hamilton straight", 0, 78);
    add_corner("T1-2", 79, 80);
    add_straight("T2-3", 81, 170);
    add_corner("T3-5", 171, 215);
    add_straight("Wellington straight", 216, 379);
    add_corner("T6-7", 380, 442);
    add_straight("T8-Old Pit Straight", 443, 616);
    add_corner("T9 Copse", 617, 618);
    add_straight("T9-T10", 619, 733);
    add_corner("T11", 734, 735);
    add_straight("T11-12", 736, 764);
    add_corner("T12", 765, 767);
    add_straight("T12-13", 768, 789);
    add_corner("T13", 790, 794);
    add_straight("T14-Hanger Straight", 795, 990);
    add_corner("T15 Stowe", 991, 1002);
    add_straight("Vale Straight", 1003, 1083);
    add_corner("T16-17", 1084, 1117);
    add_straight("T18", 1118, 1160);


    // // TODO: every Straight below needs real (start_index, end_index) pairs into
    // // qss_silverstone.csv instead of the old (length[, sm, sm_start, sm_end]) values --
    // // commented out until that QSS-index lookup is done for each one. FastCorner/SlowCorner
    // // are untouched (out of scope for the Segment/Straight/Corner index-based refactor).
    // // track.push_back(std::make_unique<Straight>("Hamilton Straight             ", 410, true, 25, 325));
    // track.push_back(std::make_unique<FastCorner>("Abbey, Turn 1                 ", 101, 106, 307, 295, 100));
    // track.push_back(std::make_unique<FastCorner>("Farm, Turn 2                  ", 82, 170, 286, 263, 100));

    // track.push_back(std::make_unique<SlowCorner>("Turn 3                      ", 110, 74, 5.04, 263, 108, 141, 50));
    // track.push_back(std::make_unique<SlowCorner>("Turn 4                      ", 68, 79, 5.06, 141, 87, 184, 50));
    // track.push_back(std::make_unique<FastCorner>("Turn 5                      ", 108, 53, 237, 257, 100));

    // // track.push_back(std::make_unique<Straight>("Wellington Straight           ", 585, true, 0, 585));

    // track.push_back(std::make_unique<SlowCorner>("Turn 6                      ", 135, 88, 4.13, 280, 167, 184, 43));
    // track.push_back(std::make_unique<SlowCorner>("Turn 7                      ", 74, 145, 6.17, 284, 114, 230, 50));

    // // track.push_back(std::make_unique<Straight>("Woodcote Turn 8 - Old Pit Straight", 690, true, 0, 590));

    // track.push_back(std::make_unique<FastCorner>("Copse, Turn 9                 ", 109, 56, 284, 287, 100));

    // // track.push_back(std::make_unique<Straight>("Turn 9 - Turn 10 Straight ", 384.5));

    // track.push_back(std::make_unique<FastCorner>("Maggots, Turn 10              ", 75.5, 46, 285, 0, 100));
    // track.push_back(std::make_unique<FastCorner>("Maggots, Turn 11              ", 45, 93, 278, 270, 100));
    // track.push_back(std::make_unique<FastCorner>("Becketts, Turn 12             ", 77, 54, 259, 0, 80));
    // track.push_back(std::make_unique<FastCorner>("Becketts, Turn 13             ", 112, 68.5, 219, 244, 55));

    // // track.push_back(std::make_unique<Straight>("Turn 14 - Hanger Straight     ", 813.5, true, 90.5, 813.5));

    // track.push_back(std::make_unique<SlowCorner>("Stowe, Turn 15                ", 132.5, 959, 3.38, 290, 236, 249, 64));

    // // track.push_back(std::make_unique<Straight>("Vale Straight                 ", 290.5));

    // track.push_back(std::make_unique<SlowCorner>("Turn 16                     ", 65, 30.5, 3.16, 260, 105, 113, 40));
    // track.push_back(std::make_unique<FastCorner>("Turn 17                     ", 36.5, 63, 130, 184, 40));
    // track.push_back(std::make_unique<FastCorner>("Turn 18                     ", 113, 51, 226, 243, 100));
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
    index = (index + 1) % static_cast<int>(track.size());
}

// Move to previous next segment
void Track::decre(){
    index = (index + static_cast<int>(track.size()) - 1) % static_cast<int>(track.size());
}

// Reset to the first segment
void Track::reset(){
    index = 0;
}

// Return the number of segments in the track
int Track::size(){
    return static_cast<int>(track.size());
}

// Return the current index
int Track::get_index(){
    return index;
}

void Track::add_straight(std::string name, size_t start_index, size_t end_index){
    auto straight = std::make_unique<Straight>(name, start_index, end_index);
    straight->parse_data(track_data, curvature);

    track.push_back(std::move(straight));
}

void Track::add_straight(std::string name, size_t start_index, size_t end_index, double sm_start, double sm_end){
    auto straight = std::make_unique<Straight>(name, start_index, end_index, sm_start, sm_end);
    straight->parse_data(track_data, curvature);

    track.push_back(std::move(straight));
}

void Track::add_corner(std::string name, size_t start_index, size_t end_index){
    auto corner = std::make_unique<Corner>(name, start_index, end_index);
    corner->parse_data(track_data, curvature, qss);

    track.push_back(std::move(corner));
}


