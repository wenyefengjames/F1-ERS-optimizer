#include "../include/optimizer.h"
#include <iostream>

// g++ -std=c++20 .\test\optimizer_test_file.cpp .\src\optimizer.cpp .\src\car.cpp .\src\battery.cpp .\src\physics.cpp .\src\track-model\segment.cpp .\src\track-model\track.cpp .\src\track-model\straight.cpp .\src\track-model\fast-corner.cpp .\src\track-model\slow-corner.cpp .\src\track-model\corner.cpp -o test_optimizer.exe
// .\test_optimizer.exe

int main(){
    Optimizer opt(false, false);

    // option_table_straight_new() static_casts both circuit neighbours of the straight to
    // Corner*. Track::Track()'s real circuit still builds the old FastCorner/SlowCorner types,
    // so calling this on any of the 18 real segment indices right now is undefined behaviour
    // (invalid downcast), unrelated to whether the braking-table math itself is correct.
    // Appending a fresh Corner -> Straight -> Corner trio here sidesteps that without touching
    // Track::Track() -- the middle Straight's prev()/next() resolve to these two real Corner
    // objects, not to the wraparound edge of the real circuit.
    std::vector<SpeedTraceType> dummy_trace;

    opt.circuit.add_corner("Test Corner In", 100, 200, 3.0, 50, 0, dummy_trace);
    opt.circuit.add_straight("Test Straight", 500);
    opt.circuit.add_corner("Test Corner Out", 150, 250, 3.0, 50, 0, dummy_trace);

    int straight_index = opt.circuit.size() - 2;

    std::cout << "Testing segment: " << opt.circuit.at(straight_index)->get_name() << "\n";
    std::cout << "===========================================\n";

    std::vector<Option> options = opt.option_table_straight_new(straight_index);

    std::cout << "===========================================\n";
    // std::cout << "Number of options generated: " << options.size() << "\n";
    // for(const Option& op : options){
    //     std::cout << "Deploy: " << op.deploy << " MJ\t";
    //     std::cout << "Harvest: " << op.harvest << " MJ\t";
    //     std::cout << "Delta time: " << op.delta << " s\n";
    // }
}
