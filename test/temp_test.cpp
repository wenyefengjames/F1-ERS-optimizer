#include "../include/track.h"
#include <iostream>

// g++ -std=c++20 .\test\temp_test.cpp .\src\track-model\track.cpp .\src\track-model\segment.cpp .\src\track-model\straight.cpp .\src\track-model\corner.cpp .\src\track-model\track-generation.cpp .\src\physics.cpp -o temp_test.exe
// .\temp_test.exe

int main(){
    Track track;

    for(int i = 0; i < track.size(); i++){
        Segment* seg = track.at(i);

        std::cout << "===========================================\n";
        std::cout << "Name: " << seg->get_name() << "\n";
        std::cout << "Start index: " << seg->get_start_index() << "\t";
        std::cout << "End index: " << seg->get_end_index() << "\n";
        std::cout << "Length: " << seg->get_length() << " m\n";

        if(seg->get_type() == SegmentType::Straight){
            auto straight = static_cast<Straight*>(seg);
            std::cout << "Type: Straight\n";
            std::cout << "SM: " << straight->get_sm() << "\t";
            std::cout << "SM start: " << straight->get_sm_start() << "\t";
            std::cout << "SM end: " << straight->get_sm_end() << "\n";
        }
        else if(seg->get_type() == SegmentType::Corner){
            auto corner = static_cast<Corner*>(seg);
            std::cout << "Type: Corner\n";
            std::cout << "Entry speed: " << corner->get_entry_speed() << " km/h\t";
            std::cout << "Exit speed: " << corner->get_exit_speed() << " km/h\n";
            std::cout << "Time: " << corner->get_time() << " s\t";
            std::cout << "Energy: " << corner->get_energy() << " J\n";

            auto trace = corner->get_speed_trace();
            std::cout << "Speed trace (" << trace.size() << " points):\n";
            for(const auto& point : trace){
                std::cout << "  Speed: " << point.speed_kmh << " km/h\t";
                std::cout << "Distance: " << point.distance_m << " m\n";
            }
        }
        else{
            std::cout << "Type: (unexpected -- FastCorner/SlowCorner shouldn't appear in the active circuit)\n";
        }
    }

    std::cout << "===========================================\n";
    std::cout << "Total segments: " << track.size() << "\n";
}
