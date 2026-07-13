#include "../../include/track.h"

Track::Track(){
    track.push_back(std::make_unique<Straight>("Hamilton Straight", 458, 0.0));
    track.push_back(std::make_unique<FastCorner>("Abbey, Turn 1", 0.0, 0.0, 300, 300, 100));
    track.push_back(std::make_unique<FastCorner>("Farm, Turn 2", 0.0, 0.0, 300, 300, 100));
    track.push_back(std::make_unique<SlowCorner>("Turn 3 - 5", 0.0, 12.3, 87, 240, 50));
    track.push_back(std::make_unique<Straight>("Wellington Straight", 595, 0.0));
    track.push_back(std::make_unique<SlowCorner>("Turn 6 - 8", 0.0, 10.37, 114, 230, 100));
    track.push_back(std::make_unique<Straight>("Woodcote - Old Pit Straight", 667, 0.0));
    track.push_back(std::make_unique<FastCorner>("Copse, Turn 9", 0.0, 0.0, 282, 282, 100));
    track.push_back(std::make_unique<Straight>("Turn 9 - Turn 10", 476, 0.0));
    track.push_back(std::make_unique<FastCorner>("Maggots, Turn 11", 0.0, 0.0, 282, 282, 100));
    track.push_back(std::make_unique<FastCorner>("Becketts, Turn 12", 0.0, 0.0, 259, 259, 100));
    track.push_back(std::make_unique<FastCorner>("Becketts, Turn 13", 0.0, 0.0, 219, 244, 70));
    track.push_back(std::make_unique<Straight>("Turn 14 - Hanger Straight", 785, 0.0));
    track.push_back(std::make_unique<SlowCorner>("Stowe, Turn 15", 0.0, 3.37, 236, 249, 100));
    track.push_back(std::make_unique<Straight>("Vale straight", 286, 0.0));
    track.push_back(std::make_unique<SlowCorner>("Turn 16 - 18", 0.0, 9.24, 102, 241, 100));
}
        


