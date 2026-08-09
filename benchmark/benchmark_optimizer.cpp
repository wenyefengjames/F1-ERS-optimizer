#include <benchmark/benchmark.h>
#include "../include/optimizer.h"

Optimizer glob = Optimizer(true, true);
double initial_battery = 4;

// IMPORTANT: Because the index values in this file are all hardcoded values,
//            if more details are added to Track.cpp, we need to change this file
//            to make it work properly

static void BM_OptionTableLookUpInitialize(benchmark::State& state) {
    for(auto _ : state){
        Optimizer opt = Optimizer(true, true);
        benchmark::DoNotOptimize(&opt);
    }
}
BENCHMARK(BM_OptionTableLookUpInitialize);

// ==============================================
// option_table_straight()
// ==============================================

static void BM_OptionTableStraight_HamStraight(benchmark::State& state){
    int index = 0;

    for(auto _ : state){
        auto result = glob.option_table_straight(index, initial_battery);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_OptionTableStraight_HamStraight);

static void BM_OptionTableStraight_WellStraight(benchmark::State& state){
    int index = 6;

    for(auto _ : state){
        auto result = glob.option_table_straight(index, initial_battery);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_OptionTableStraight_WellStraight);

static void BM_OptionTableStraight_WoodcoteOPS(benchmark::State& state){
    int index = 9;

    for(auto _ : state){
        auto result = glob.option_table_straight(index, initial_battery);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_OptionTableStraight_WoodcoteOPS);

static void BM_OptionTableStraight_T9T10Straight(benchmark::State& state){
    int index = 11;

    for(auto _ : state){
        auto result = glob.option_table_straight(index, initial_battery);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_OptionTableStraight_T9T10Straight);

static void BM_OptionTableStraight_HangStraight(benchmark::State& state){
    int index = 16;

    for(auto _ : state){
        auto result = glob.option_table_straight(index, initial_battery);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_OptionTableStraight_HangStraight);

static void BM_OptionTableStraight_ValeStraight(benchmark::State& state){
    int index = 18;

    for(auto _ : state){
        auto result = glob.option_table_straight(index, initial_battery);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_OptionTableStraight_ValeStraight);

// ==============================================
// option_table_fastcorner()
// ==============================================

static void BM_OptionTableFastCorner_T1(benchmark::State& state){
    int index = 1;

    for(auto _ : state){
        auto result = glob.option_table_fastcorner(index, initial_battery);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_OptionTableFastCorner_T1);

static void BM_OptionTableFastCorner_T2(benchmark::State& state){
    int index = 2;

    for(auto _ : state){
        auto result = glob.option_table_fastcorner(index, initial_battery);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_OptionTableFastCorner_T2);

static void BM_OptionTableFastCorner_T9(benchmark::State& state){
    int index = 10;

    for(auto _ : state){
        auto result = glob.option_table_fastcorner(index, initial_battery);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_OptionTableFastCorner_T9);

static void BM_OptionTableFastCorner_T10(benchmark::State& state){
    int index = 12;

    for(auto _ : state){
        auto result = glob.option_table_fastcorner(index, initial_battery);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_OptionTableFastCorner_T10);

static void BM_OptionTableFastCorner_T11(benchmark::State& state){
    int index = 13;

    for(auto _ : state){
        auto result = glob.option_table_fastcorner(index, initial_battery);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_OptionTableFastCorner_T11);

static void BM_OptionTableFastCorner_T12(benchmark::State& state){
    int index = 14;

    for(auto _ : state){
        auto result = glob.option_table_fastcorner(index, initial_battery);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_OptionTableFastCorner_T12);

static void BM_OptionTableFastCorner_T13(benchmark::State& state){
    int index = 15;

    for(auto _ : state){
        auto result = glob.option_table_fastcorner(index, initial_battery);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_OptionTableFastCorner_T13);



// ==============================================
// option_table_slowcorner()
// ==============================================

static void BM_OptionTableSlowCorner_T3(benchmark::State& state){
    int index = 3;

    for(auto _ : state){
        auto result = glob.option_table_slowcorner(index);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_OptionTableSlowCorner_T3);


// ==============================================
// Algorithms that runs the program
// ==============================================

static void BM_DPAlgorithm(benchmark::State& state){
    for(auto _ : state){
        state.PauseTiming();
        Optimizer opt = Optimizer(false, true);
        Battery batt = Battery(4, 0, false, true);
        state.ResumeTiming();

        auto result = opt.dp_algorithm(0, batt, 0);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_DPAlgorithm);


static void BM_PathReconstruction(benchmark::State& state){
    Battery batt = Battery(initial_battery, 0, false, true);
    Optimizer opt = Optimizer(false, true);
    double time = opt.dp_algorithm(0, batt, 0);

    for(auto _ : state){
        auto result = opt.path_reconstruction(0, initial_battery, 0, 0);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_PathReconstruction);