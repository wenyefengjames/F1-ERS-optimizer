#include "../include/track-generation.h"
#include <iostream>


namespace t = track_gen;

int main(){

    auto qss = t::qss("");
    auto read_csv = t::read_csv("silverstone_antonelli_quali.csv");
    auto compute_curvature = t::compute_curvature(read_csv);
    auto compute_vmax = t::compute_vmax(compute_curvature);

    t::write_csv();

    // for(size_t i = 0; i < qss.size(); i++){
    //     std::cout << "QSS speed: " << qss[i] * 3.6 << '\t';
    //     std::cout << "CSV distance: " << read_csv[i].distance_m << '\t';
    //     std::cout <<  "x position: " << read_csv[i].x_pos << '\t';
    //     std::cout <<  "y position: " << read_csv[i].y_pos << '\t';
    //     std::cout << "compute_curvature: " << compute_curvature[i] << '\t';
    //     std::cout << "compute_vmax: " << compute_vmax[i] * 3.6 << '\n';
    // }

    // g++ -std=c++20 .\test\track-generation-test.cpp .\src\track-model\track-generation.cpp .\src\physics.cpp -o test_track_gen.exe
}


