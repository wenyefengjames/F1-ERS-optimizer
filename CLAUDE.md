# Project: F1 Battery Deployment Optimizer

## Goal
A C++ project simulating and optimizing battery (MGU-K) deployment across a lap,
targeting 2026 F1 regulations. Built to strengthen my CV for Red Bull (CFD Software
Engineering) and Alpine (Software Engineering placement) applications. We are only focusing on the silverstone race track for this software for now. If there is time more tracks can be added in the future. Isn't a priority for now

## Domain context — 2026 F1 power unit regulations
- Battery: has a maximum charge of 4MJ usable energy cap at any time. No MGU-H — MGU-K only (simpler single source/sink energy balance vs. the old car).
- MGU-K: Maximum rate of deploy and recover capped at 350kW.
- Deployment: up to 4MJ bursts per deployment, multiple deployments per lap allowed if battery supports it (old rules allowed one deployment window/lap). Deployment tapers above 290km/h, hits zero at 355km/h (leading car). A following car within 1s gets MOM, which allows full 350kW up to 337km/h. Note: there are no publicly confirmed taper curve shape, so we assume as linear interpolation. 
- Harvest per lap (Silverstone, 2026 British GP): regulated per-session cap,
  not a physical estimate — qualifying ≈ 6.5MJ, race ≈ 8.0MJ. Qualifying is the
  circuit-sensitive number (FIA cuts it per-event when a track doesn't offer
  enough natural braking energy); race stays close to a flat ~8MJ baseline
  across circuits. Model these as two separate input parameters
  (qualifying_harvest_cap_MJ / race_harvest_cap_MJ), not one shared constant. When the trailing car gets MOM, they are allowed 0.5MJ more harvest in a lap than the car in front.
- Harvest methods: braking, partial throttle, coasting, "superclipping"
  (diverting engine power to battery at full throttle). Cap: 350kW as of the
  Miami GP mid-season update (was 250kW at season start).
- 1hp = 0.7457kW


## Target circuit: Silverstone
Chosen deliberately — few heavy braking zones (long, fast, flowing corners like
Maggotts-Becketts-Chapel) means harvesting opportunities are scarce, making
energy management a genuinely tight, interesting problem rather than a simple
"brake a lot, deploy on straights" model.

## Core approach
- Lap modeled as a sequence of discrete track segments (straight/slow corner/fast corner),
  each with a distance, time. Corners have apex-speed, exit-speed, throttle attibutes to estimate laptime and energy demand/harvest potential.
- Battery state modeled with the real constraints above.
- Car and Battery are split into two classes: `Battery` owns charge state, the
  per-lap harvest limit, and the race/qualifying mode switch; `Car` owns
  physical constants (ICE/MGU-K power, mass) and the deployment physics
  (taper curve, kinetic-energy↔speed conversion), and holds a `Battery` as a
  member.
- Optimizer: dynamic programming over discretized battery states (~0.1MJ steps)
  across segments — backward induction to find minimum lap time per
  (segment, battery-level) pair.
- Three modes, same DP core with different constraints:
  - Qualifying: single-lap horizon, no carry-over constraint.
  - Race: multi-lap, battery state carries over, per-lap harvest cap enforced.
  - Attack/defend: takes a gap-to-car time trace as input; decides when to
    trigger MGU-K Override (attack) or prioritize deployment before likely
    attack zones (defend).
- Real lap data (speed/throttle/brake traces) via FastF1 (Python) for the actual
  2026 British GP at Silverstone, used to derive segment energy potential —
  battery-specific numbers are estimated/derived, not proprietary team data
  (which isn't public).

## Timeline
- **Days 1-9 (current phase): core working prototype.** DONE
  Segment + battery data model, DP optimizer (race + qualifying modes), basic
  CLI to run a lap and compare against a naive baseline. This is what goes in
  my CV/application — needs to be genuinely working, not just scaffolding.
- **Following 1-2 months: full project.**
  Attack/defend mode, Python data pipeline (FastF1) integration, Google Test/
  Catch2 unit tests, GitHub Actions CI (build+test+clang-tidy+sanitizers),
  REST API wrapper, Docker, and if time allows: visualization (Qt or simple
  chart via Python bindings) and a Kubernetes manifest.

## Stack (update as decisions are made)
- Build: CMake + Ninja
- Compiler: GCC 14.2.0 (Windows)
- Language standard: C++20
- header/.h files live in include/, source/.cpp files live in src/
- In physics.cpp, physics work in Joules. In Battery class, energy is stored as MJ
- Testing: Google Test
- Debugging: Clang-tidy, Sanitizer
- Performance Benchmarking: Google Benchmark
- Later: FastF1 (Python) for data, pybind11 for C++/Python bridge (tentative),
  GitHub Actions for CI, Docker

## Working style — IMPORTANT
I am implementing all code myself to build real understanding for interviews.
Act as a senior engineer doing code review, not as an implementer:
- Point out design issues, missing edge cases, bugs, and non-idiomatic C++.
- Ask me to explain my reasoning on non-obvious decisions.
- Suggest better approaches, but do NOT write or rewrite implementation code
  for me unless I explicitly ask you to write something.
- If I ask "how would you approach X," explain the approach conceptually first
  — let me write the implementation myself.

## Progress log
(Keep this updated — what's actually built, not just planned)
- [x] Project skeleton (CMake + Ninja building)
- [x] Segment data model (Segment base + Straight/SlowCorner/FastCorner)
- [x] Battery state model
- [x] Car / physics model — ICE/MGU-K/mass constants, deployment, taper curve, kinetic-energy↔speed conversion, harvesting methods (braking/coasting/superclip/partial-throttle)
- [x] Drag-aware physics formulas (physics.h/.cpp) — kinetic energy, work done under power with drag, required power, distance-to-recharge, time-to-reach-velocity. Validated through GoogleTest unit tests.
- [x] DP optimizer core, consider how to optimize recharge, how much hp goes to superclipping etc..
- [x] Generating a table of (battery, delta) pairs for each segment of the track (`segment_options()`), so that the DP optimizer can use this information to decide the best course of action. For each case of Straight, FastCorner and SlowCorner, the functions have been implemented.
- [x] Qualifying mode — manually tested across several input combinations; harvest-cap enforcement (via the `harvest_charge` DP dimension) confirmed correctly respected.
- [ ] Race mode (multi-lap) — partial: the DP supports race-mode parameters (the race harvest-cap variant, and an arbitrary starting/ending battery for a single lap), but doesn't yet simulate multiple laps with battery/harvest state carrying over lap-to-lap as the original spec above describes (`main_optimizing_loop` only ever runs one lap; nothing calls `Battery::reset_harvest()` between laps because there's no lap loop yet).
- [x] MVP completed
- [x] GoogleTest set up; unit tests written for `Battery`, `Car`, and `Physics` (simple/linear formulas + documented edge-case behavior for all of them). The transcendental drag-ODE formulas (`work_done_with_drag`, `required_power`, `distance_to_recharge`, `time_to_reach_velocity`) deliberately have only boundary/round-trip property tests, not exact-value ones — they're about to be rewritten with numerical methods for tapering, so locking in hand-typed constants for the current closed-form versions isn't worth it. (`coasting_energy_loss` was later removed entirely — its functionality was exactly `work_done_with_drag(0, ...)` with a sign flip, so it added no coverage of its own.)
- [x] Unit test `Track` (segment data + `next()`/`prev()` wraparound) and `Optimizer::index_helper` (the flattened-index calculation that's caused the most repeat bugs this session — will need `FRIEND_TEST` since it's private). No DP integration tests yet — blocked on `Optimizer` hardcoding its own `Track` member, so there's currently no way to inject a small fake track to hand-verify DP output against.
- [x] clang-tidy + sanitizer builds (from the original CI plan below) — cheap to wire up now that a test suite exists to run under them; likely to catch more of the same bug class as the `Battery` constructor's uninitialized `harvest_limit` read found earlier this session.
- [x] Implement and bring in tapering function, alongside reworking the closed-form drag formulas in `physics.cpp` to numerical methods (a speed-dependent taper curve doesn't have a clean closed-form integral).
- [x] Performance: fix `segment_options()`'s redundant recomputation across `ending_battery`/`harvest` combinations that reach the same `(index, battery_charge)` state (known, deferred). Do this *after* the physics renovation above, not before — optimizing formulas that are about to be rewritten is wasted effort. This is fixed by caching pre-computed option tables as a lookup table.

## MVP (ie the core prototype after 9 days)
a CLI that takes no arguments (or minimal ones), builds the Silverstone segment list internally, runs the DP(or whatever optimization algorithm I make) for qualifying mode and race mode, and prints something like a per-segment table showing battery level and deployment decision, the change in delta when we decide to deploy/recharge at each segment, and total lap time — ideally alongside a naive baseline that we can compare to (e.g. "always deploy fully when possible", "never deploys", or "deploys the same amount in every segment") so the DP's improvement is visible and quantifiable, and we can compare the overall improvement in delta as well. More specific definitions and specifications can be found in PROJECT_SPEC.md.

## The Second prototype (Deadline: 7th of August)
What should the second prototype look like:
- A more refined track representation
- More realistic numerical physics equations
- Optimizer takes into the account that with MOM we get 0.5 MJ more harvest per lap 

How to achieve each one ->

[x] Track representation:
- [x] Change the structure of Segment, Straight, FastCorner and SlowCorner classes to accomodate the new details and changes
- [x] The length of every corner and straight measured
- [x] The distance from corner entry to corner apex varies between corners, therefore more accurate representation of breaking
- [x] Straight Mode included in straights that have them. Be aware of different starting positions of SM


[x] Physics model:
- [x] Make every function Taper aware -- with one deliberate exception: `work_done_with_drag`, `distance_to_recharge`, and `time_to_reach_velocity` stay taper-agnostic on purpose (plain constant power in, no `taper_curve` knowledge). Tapering is applied by whichever function calls them repeatedly with recomputed power instead (`energy_deployed_with_taper` `time_to_reach_speed_over_distance`, `build_taper_table`).
- [x] Seperate the Drag Coefficient calculation from the drag formula, because it can vary in the future independent of speed
- [x] Straight Mode aware, as that reduces the Drag Coefficient -- `sm_on` now threaded through every drag-involving function.
- [x] MOM aware, can reach a higher top-speed before Taper kicks in -- `mom` threaded through `time_to_reach_speed_over_distance`, `energy_deployed_with_taper`, and the taper table functions.
- [x] Numerical method, because integration won't work anymore -- `energy_deployed_with_taper`, `build_taper_table`, and `time_to_reach_speed_over_distance`'s accelerating branch all step forward numerically now.
- [x] Lookup table for Tapering, improve efficiency -- `build_taper_table` / `taper_table` / `search_taper_table`.
- [x] Fix the problem that the time function doesn't give a reasonable output when the difference in speed is small -- fixed for the case that actually mattered (`vi == vf` exactly), via `time_to_reach_speed_over_distance`'s dedicated cruise branch, which never touches the degenerate antiderivative for that case. Near-equal-but-not-exactly-equal speeds were also manually stress-tested (`temp_test.cpp`, sweeping the gap down to ~0.001km/h) at the specific power where this is riskiest -- a tiny gap implies a "hold speed constant" power, whose terminal velocity sits right on the antiderivative's `log|a-vel|` singularity -- and the result converged smoothly with no instability. So the only genuinely broken case is exact equality, which is already handled separately.

[x] Optimizer:
- [x] Integrate the new features of Physics correctly
- [x] Integrate the new features of Track correctly
- [x] Change the harvest limit depending on having MOM or not
- [x] Fix the bug that if I enter 0 starting battery and 0 ending battery, the output of laptime is inf (Found the bug, starting with 0 battery means no deployment in Hamilton Straight, can't reach the target speed of Turn 1 with just ICE. Haven't implemented the feature where the speed reaching fast corner's apex speed doesn't need to be exactly the speed. Therefore not fixable for now)

Bug fixes:
- [x] Use clang-tidy improve coding quality
- [x] Optimizer: Fix the bug that if I enter 0 starting battery and 0 ending battery, the output of laptime is inf (Found the bug, starting with 0 battery means no deployment in Hamilton Straight, can't reach the target speed of Turn 1 with just ICE. Haven't implemented the feature where the speed reaching fast corner's apex speed doesn't need to be exactly the speed. Therefore not fixable for now)
- [x] Physics: Fix the bug that the time function doesn't give a reasonable output when the difference in speed is small
- [x] Performance: fix `segment_options()`'s redundant recomputation across `ending_battery`/`harvest` combinations that reach the same `(index, battery_charge)` state (known, deferred). (This will be fixed when the feature of each segment option table gets cached is implemented, which would be a bigger performance improvement than this)

Limitation:
- Fastcorners like T1 and T2 should have apex_min_speed as the maximum speed they could achieve, the optional table should provide options where more recharging can be done to go through the corner in a lower speed than apex_min_speed. Results in a increase in delta but harvest more energy
- SlowCorners still rely on a fixed entry speed to calculate breaking energy. In the future, where to break to harvest the best can be calculated by the optimizer.
- In best_option_for_bucket(), the harvest-phase will use the Straight Mode from the start of the harvesting all the way till the end. Even if we cross the part where SM should close

## Third Prototype: (Deadline: 14th of August)
What it should look like:
- Performance optimization
- Accurate track data that is very generalized. I.e. Can easily import multiple different tracks
- Visualized Speed Trace. Plotted speed over distance, deployment over distance etc
- Increased physics inputs. E.g. modifiable weight, tire grip etc
- Refined physics models, turbulant air awareness, which impacts laptime and downforce
- Add a new category for Segment to represent corners like T5, T17 and T18. 
- Bring in MOM detection point

[x] Performance
- [x] Computing each Segment's option tables once and cache it. Then during DP it wouldn't need to compute them anymore. 
- [x] Install Google Benchmark
- [x] Go through track models and minimize unnecessary memory copies
- [x] After installing Google Benchmark. Re-run the old no cache dp algorithm and compare to the new cached dp algorithm

[ ] Optimizer
- [ ] Slow corners: The breaking distance, and entry speed can vary
- [ ] Fast corners: We have apex_min_speed as a cap to the max speed we can go through. We can try a number of different speeds 
- [ ] Straight: the deployment doesn't have to hard code as 350kW from MGU-K. We can try deploy in different amount, like step_size in the exits of fast corner. We also doesn't have to keep it constant. Could deploy max for 100m, then 200kW for another 50. Then 0, then recharge etc.

[ ] Track model
- [ ] Use curvature-based track layouts. Using real (x,y) position telemetry through FastF1. 
- [ ] Curvature can derive speed limits.


[ ] Refined Physics
- [ ] Add a parameter to drag_coeff() that tells the time difference of the car infront, this should determine the drag and also affect downforce
- [ ] A function should return how much will the laptime get affected depending on how close we are to the car in front. This can change the time through a slow corner by a percentage, reduce the exit speed and apex min speed through a fast corner. etc
- [ ] Research about Vehicle Dynamics. Then update this part of the progress log with things I can implement (E.g. tire grip, fuel load weight)





## Fourth Prototype: (Deadline: 21st of August)
What the third prototype should look like:
- Multiple car simulation (Complete Race Mode)
- Attacking mode
- Defending mode
- Multithreading (Maybe)
- Make the data about silverstone circuit into a seperate file, so that in the future where if we have the same structure of track data from other circuit, we can easily integrate it into our optimizer.
- With Slow corners, we have a baseline of the fastest time to get through the corners (referenced in qualifying), but it can be very damaging to the tires. So we can look at telementry and onboards to figure out how much drivers have slown down to manage tired and benefit in the long run. 

[ ] Performance
- [ ] Multithreading, threadpool (Instead of using mutex to write into the same memory space. Since The option table lookup table has a separate cell for each segment. Each thread could handle one segment by itself without worrying about writing into the same memory at all. Which can be a better multithreading implementation, disjoint memory regions > mutex)

[ ] Race Mode
- [ ] This might be a bigger DP choice over multiple laps over the small DP algorithm.
- [ ] Input Parameters can be: Aiming to gain X seconds over Y laps. With a starting battery of A and resulting battery of B. 

Limitations: 
- [ ] MOM detection point at the end of T17. Probably not something to implement in track model but be aware
- [ ] Be aware that Hamilton Straight starts 50m before the S/F line of the next lap. There is 50m more for the car to travel after finishing the last corner (T18) 
- [ ] When deploying energy on the straight, change from a fixed max deployment of 350kW from MGU-K, to trying different values of deployment, for now it can be a fixed step-size of 25kW or 50kW (After the optimization of caching segment tables is done, this adds a lot of computation power)

## Fourth Prototype:
- Implement different models to represent the downforce and aero package of cars from different teams (Because, if its a slower car in front, we could deploy less to get pass, which saves battery)
- Implement different tire models that gives different grip, which impacts laptime
