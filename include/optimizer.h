#pragma once
#include "car.h"
#include "track.h"
#include "physics.h"
#include <optional>
#include <vector>

// This struct holds the value pair of battery cost with respect to delta time
struct Option {
    double deploy;
    double harvest;
    double delta;
};

// This struct holds the information of how the deployment and harvesting is actually executed
struct ExecutionDetails{
    double deployment_distance_m;
    double deployment_rate_kW;
    double harvest_distance_m;
    double harvest_rate_kW;
    double braking_distance_m;
};

class Optimizer{

    private:
        const int partition_size = 10;          // The length that distance of segment needs to be divided by
        const double bucket_size = 0.1;        // The size of energy bucket being used in each step, in MJ
        const int battery_buckets = 4.0/bucket_size + 1;    // Number of buckets that the battery is divided into
        const double deployment_step_size = 50; // in kW, how much increase for each step of deployment
        bool race_mode;
        bool mom;
        // Track circuit = Track(); // TO DO REMOVE THIS COMMENT AFTER TESTING
        Car car = Car(race_mode, mom);
        const int harvest_buckets = car.get_battery().get_harvest_limit() / bucket_size + 1;
        std::vector<double> table;                      // Table for memoization
        std::vector<std::optional<Option>> choice;      // Table for path reconstruction
        std::vector<std::vector<Option>> option_table_lookup_table;     // A lookup table for every segment's option table
        // std::vector<std::vector<ExecutionDetails>> execution_lookup_table;        // A lookup table for every Option's execution in each segment 

        // TO DO REMOVE THIS COMMENT AFTER TESTING
        // std::vector<Option> option_table_straight(int seg_index);
        // std::vector<Option> option_table_corner(int seg_index); 
        // void initialize_option_table_lookup_table();

    public:
        std::vector<std::vector<ExecutionDetails>> execution_lookup_table;        // A lookup table for every Option's execution in each segment 
        Track circuit = Track();
        std::vector<Option> option_table_straight(int seg_index);
        std::vector<Option> option_table_corner(int seg_index); 
        void initialize_option_table_lookup_table();

        // Helper function to flatten the 4D table into 1D by translating index positions
        unsigned int index_helper(int i, double b, double e, double h);

        Optimizer(bool race_mode, bool mom);

        double main_optimizing_loop(int seg_index, double initial_battery, double ending_battery, double harvest);
        double dp_algorithm(int index, Battery battery, double ending_battery);
        std::vector<Option> path_reconstruction(int starting_index, double battery, double ending_battery, double harvest);
        std::vector<Option> segment_options(int seg_index);

        // AI generated code to allow visualisation on results

        // ==================================================================================
        // NEW: speed-trace reconstruction for the DP's final chosen path. All added fresh,
        // nothing above this line was touched to build these.
        // ==================================================================================

        // Walks the winning path (via path_reconstruction) and builds the full speed trace:
        // Corner segments contribute their own stored, QSS-derived trace unchanged; Straight
        // segments are reconstructed via reconstruct_straight_trace(). Assumes seg_index == 0
        // (a full lap from the start line) -- the running distance offset is built by summing
        // get_length() from the very first segment visited, which only lines up with Corner's
        // own absolute track_data-based distances if that first segment really is the lap start.
        std::vector<SpeedTraceType> compute_final_speed_trace(int seg_index, double initial_battery, double ending_battery, double harvest);

        // Writes compute_final_speed_trace()'s result to data/track-data/<file_name> as a
        // Speed,Distance csv (same folder/style convention as write_csv() in track-generation.cpp).
        void write_speed_trace_csv(const std::string& file_name, int seg_index, double initial_battery, double ending_battery, double harvest);

        // Finds the ExecutionDetails entry matching a winning Option by exact value equality.
        // option_table_lookup_table and execution_lookup_table are pushed to in lockstep (same
        // order) inside option_table_straight(), so the match is expected to be exact and unique --
        // the winning Option itself is a plain copy of a value already in option_table_lookup_table,
        // never independently recomputed, so no floating-point drift is expected between them.
        ExecutionDetails find_execution_details(int seg_index, const Option& winning_option);

        // Reconstructs a Straight segment's chosen execution as a fine-grained (distance, speed)
        // trace, by re-stepping through the same physics used to generate it (deployment,
        // harvest/cruise, braking) instead of just returning the summary values ExecutionDetails
        // stores. Deliberately mirrors (does not call) option_table_straight()'s starting_speed/
        // ending_speed derivation and braking_lookup_table construction -- INCLUDING that
        // function's existing bug where the ending_speed branch reads circuit.prev(seg_index)
        // instead of circuit.next(seg_index) -- kept as-is on purpose so this trace stays
        // consistent with whatever the cached tables actually used to pick this option. See
        // chat for why that line needs fixing separately.
        std::vector<SpeedTraceType> reconstruct_straight_trace(int seg_index, const ExecutionDetails& exe, double distance_offset_m);

        // Re-steps energy_deployed_with_taper()'s core physics (same taper_curve/work_done_with_drag/
        // reverse_ke calls, same DELTA_T), recording every step instead of just a summary. Deliberately
        // skips that function's taper-table-lookup shortcut branch, so every step is always computed
        // numerically -- same underlying physics, so the result should be extremely close to but not
        // guaranteed bit-identical to what energy_deployed_with_taper() itself would have returned.
        std::vector<SpeedTraceType> replay_deployment_phase(double starting_speed_kmh, double distance_m, double deploy_rate_kW,
                                                              double sm_start, double sm_end, double distance_offset_m);

        // Re-steps time_to_reach_speed_over_distance()'s equal-speed and decelerating (Superclip)
        // branches only -- option_table_straight() already only keeps options where speed >= entry_speed,
        // so the accelerating branch never applies to a chosen Straight option and isn't replicated here.
        std::vector<SpeedTraceType> replay_harvest_phase(double starting_speed_kmh, double target_speed_kmh, double distance_m,
                                                           bool sm, double distance_offset_m);

        // Rebuilds the exact same braking_lookup_table construction used in option_table_straight()
        // (same up-stepping-from-ending_speed formula), then walks it backwards from the row matching
        // entry_speed_kmh down to ending_speed_kmh -- guarantees the same distances as whatever was
        // actually used to select this option, rather than re-deriving a reversed stepping scheme
        // independently and risking a subtly different convention.
        std::vector<SpeedTraceType> replay_braking_phase(double entry_speed_kmh, double ending_speed_kmh, double distance_offset_m);
};