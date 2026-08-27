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
    // Corner/Straight now derive entry_speed/exit_speed/length from qss_silverstone.csv via
    // parse_data(), given a [start_index, end_index] range -- there's no way to hand these
    // objects speed values directly anymore. parse_data() is currently an empty stub, so
    // these placeholder indices produce Corner objects with entry_speed=exit_speed=length=0.0
    // until parse_data() is actually implemented -- the numbers below don't mean anything yet.
    opt.circuit.add_corner("Test Corner In", 0, 1);
    opt.circuit.add_straight("Test Straight", 0, 1);
    opt.circuit.add_corner("Test Corner Out", 0, 1);

    int straight_index = opt.circuit.size() - 2;

    // execution_lookup_table is sized to circuit.size() inside the Optimizer constructor,
    // which runs before these extra segments get appended -- option_table_straight_new()
    // writes to execution_lookup_table[seg_index] with an unchecked operator[], so without
    // this resize the call below would be an out-of-bounds write.
    opt.execution_lookup_table.resize(opt.circuit.size());

    std::cout << "Testing segment: " << opt.circuit.at(straight_index)->get_name() << "\n";
    std::cout << "===========================================\n";

    std::vector<Option> options = opt.option_table_straight_new(straight_index);
    const std::vector<ExecutionDetails>& executions = opt.execution_lookup_table[straight_index];

    std::cout << "===========================================\n";
    std::cout << "Number of options generated: " << options.size() << "\n";
    std::cout << "Number of execution entries: " << executions.size() << "\n";
    for(size_t i = 0; i < options.size(); i++){
        const Option& op = options[i];
    // for(size_t i = 0; i < (options.size()/10); i++){
    //     const Option& op = options[i * 9];
        std::cout << "Deploy: " << op.deploy << " J\t";
        std::cout << "Harvest: " << op.harvest << " J\t";
        std::cout << "Delta time: " << op.delta << " s\n";

        if(i < executions.size()){
            const ExecutionDetails& exe = executions[i];
            std::cout << "  Deployment distance: " << exe.deployment_distance << " m\t";
            std::cout << "Deployment rate: " << exe.deployment_rate << " kW\t";
            std::cout << "  Harvest distance: " << exe.harvest_distance << " m\t";
            std::cout << "Harvest rate: " << exe.harvest_rate << " W\t";
            std::cout << "  Braking distance: " << exe.braking_distance << " m\n";
        }
        else{
            std::cout << "  (no matching execution entry -- index out of range)\n";
        }
        std::cout << "-------------------------------------------\n";
    }
}
