# F1 Battery Deployment Optimizer

A C++ simulator and optimizer for MGU-K battery deployment strategy across a Formula 1 lap, built against the 2026 F1 power unit regulations. Currently modeled for Silverstone only.

## Why this project exists

Under the 2026 regulations, I spotted that battery management throughout a lap becomes genuinely critical — not just for outright lap time, but for defending position against a car attacking from behind, or for picking the right moment to attack yourself. I built this project to explore and solve that problem directly.

Silverstone was chosen deliberately as the target circuit: its long, fast, flowing corners (Maggotts–Becketts–Chapel) leave very few heavy braking zones, so energy-harvesting opportunities are genuinely scarce. That scarcity is what makes energy management here a real optimization problem, rather than a trivial "brake hard, deploy on the straights" strategy.

## What it does

The lap is modeled as an alternating sequence of **grip-limited** segments (`Corner`) and **not-grip-limited** segments (`Straight`), derived from real circuit geometry rather than hand-typed values. Given that lap, the optimizer uses dynamic programming to decide *when* and *how much* battery energy to deploy or harvest at every point on track, minimizing total lap time under the real constraints of the regulations:

- 4MJ maximum usable battery charge at any time
- MGU-K deployment/recovery rate capped at 350kW
- Deployment power tapers linearly above 290km/h (337km/h with MOM / Overtake Mode active), reaching zero at 355km/h
- Per-session harvest caps (qualifying vs. race), with a +0.5MJ bonus when MOM is active in race mode
- "Straight Mode" aerodynamics reducing drag on specific straights, with their own start/end windows

It supports both qualifying (single-lap, no carry-over) and race (harvest state tracked against the per-lap cap) modes, with MOM handled as a separate, gap-to-car-ahead-dependent input. The winning strategy can be written back out as a speed-vs-distance trace for plotting against real telemetry.

## Documentation

This README covers the what and how. Two other files cover the rest:
- **`CLAUDE.md`** — the detailed, day-to-day progress log: what's actually built vs. planned, known bugs and limitations, and the prototype-by-prototype roadmap.
- **`PROJECT_SPEC.md`** — the reasoning behind specific design choices (why particular tradeoffs were made where more than one approach was reasonable).

## Track model

Rather than hand-typing each corner's length and speeds, the track model is derived from real circuit geometry:

1. **Geometry in** — TUMFTM's Silverstone centerline data (`data/track-data/Silverstone.csv`) provides the position/distance points. FastF1 supplies real qualifying telemetry (Antonelli's fastest lap) used as the validation reference.
2. **Curvature** — computed per point from the geometry (finite-difference by default; an exact periodic-cubic-spline version is also implemented for noisier data sources).
3. **QSS simulation** — a quasi-steady-state pass computes the maximum speed each point permits under the tyre friction circle, producing a theoretical speed profile for the lap.
4. **Segmentation** — points are categorized by curvature into grip-limited and not-grip-limited stretches, then verified by hand against onboard footage. Each segment is defined by a `(start_index, end_index)` pair into that data.
5. **`parse_data()`** — each segment reads its own properties back out of the QSS results at construction: `Straight` derives its length; `Corner` derives length, entry/exit speed, time taken, harvestable energy, and its own speed trace.

The one value that can't be derived this way is the Straight Mode window (where the movable aero opens and closes), since there's no public data for it — those remain hand-specified per straight.

## Architecture

- **`Segment`** (abstract base) / **`Straight`**, **`Corner`** (derived) — the track data model, owned polymorphically by `Track` via `std::vector<std::unique_ptr<Segment>>` with a virtual destructor for safe destruction through the base pointer. Each derived class implements its own `parse_data()`, since a grip-limited and a not-grip-limited segment need different things out of the same source data.
- **`Track`** — the ordered, closed-loop sequence of segments making up a lap (currently Silverstone), with wraparound `next()`/`prev()` traversal. Owns the parsed track data, curvature, and QSS results, computed once at construction.
- **`track_gen`** namespace — the data pipeline: CSV reading, curvature computation, QSS simulation, and CSV writing.
- **`Battery`** — owns charge state, the per-lap harvest limit, and the qualifying/race/MOM mode switch.
- **`Car`** — physical constants (ICE/MGU-K power, mass), holds a `Battery`.
- **`physics`** namespace — stateless drag/downforce/tyre/kinetic-energy/taper formulas. Includes a friction-circle tyre model (lateral and longitudinal grip traded against each other, capped at 6g to approximate load-dependent friction desaturation), speed-dependent downforce with separate corner-mode and straight-mode coefficients, and acceleration/deceleration limits derived from whichever of tyre grip or engine power actually binds. The drag ODEs are solved numerically (Euler integration) since a speed-dependent taper curve has no clean closed-form integral; a precomputed lookup table (`taper_table` / `search_taper_table`, the latter a template taking a field-accessor lambda so one binary search covers all four fields) avoids re-deriving the same taper trajectory on every call.
- **`Optimizer`** — the DP core. Backward induction over a discretized state space of (segment, battery level, target ending battery, harvest so far), memoized into a flattened table. Each segment's feasible (deploy, harvest, Δtime) options are precomputed once and cached:
  - `option_table_straight()` sweeps deployment distance × deployment rate × braking-table rows, splitting each candidate into a deployment phase, a harvest/cruise phase, and a braking phase sized to arrive at the next corner's real entry speed.
  - `option_table_corner()` derives harvest options from the energy the corner's own QSS-derived trace makes available.
  - A parallel `ExecutionDetails` table records *how* each option is executed (distances and rates per phase), which is what lets the winning path be replayed back into a full speed trace afterwards.

## Project layout

```
include/            Public headers for every class above
src/                Implementations
src/track-model/    Segment classes and the track-generation pipeline
src/python-files/   FastF1 telemetry pull and result plotting
data/track-data/    Circuit geometry, QSS output, and optimizer speed traces (CSV)
test/               GoogleTest suite
benchmark/          Google Benchmark suite
```

## Building

Requires CMake 3.16+, a C++20 compiler (developed against GCC 14.2.0 via MSYS2 on Windows), and Ninja.

```
cmake -B build -G Ninja
cmake --build build
```

For anything performance-sensitive — benchmarking, comparing before/after an optimization — build in Release mode instead. An unoptimized build can be an order of magnitude slower and isn't representative of real performance:

```
cmake -B build-release -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-release
```

Run the optimizer (interactive — prompts for mode, MOM, and starting/ending battery/harvest):
```
./build/bin/ERSOptimizer
```

Run the test suite:
```
./build/bin/unit_tests
```

Run the benchmark suite (from a Release build, to get meaningful numbers):
```
./build-release/bin/benchmarks
```

AddressSanitizer + UndefinedBehaviorSanitizer builds are available via `-DENABLE_SANITIZERS=ON`.

The Python side (FastF1 telemetry, plotting) is separate from the CMake build:
```
python src/python-files/plot-data-test.py
```
This plots the optimizer's exported strategy (`data/track-data/battery_deployment_silverstone.csv`, written by a completed optimizer run) against the real telemetry line pulled live from FastF1.

## Testing and tooling

- **GoogleTest** — hand-computed exact-value tests for simple/linear formulas, and property-based tests (boundary values, monotonicity, round-trip invariants) for the transcendental drag/taper ODEs, where a hand-typed expected value would just re-derive the implementation rather than check it. Also covers `Track` traversal/wraparound, the track-generation pipeline, and `Optimizer::index_helper` (the flattened-index calculation, historically the most bug-prone piece).
- **Google Benchmark** — used to validate that performance work (segment-option-table caching, state-space discretization tuning) actually helps, rather than assuming it does.
- **clang-tidy** — configured for `bugprone-*`, `performance-*`, `modernize-*`, and `clang-analyzer-*` checks (see `.clang-tidy`).
- **AddressSanitizer / UndefinedBehaviorSanitizer** — via the `ENABLE_SANITIZERS` CMake option above.

## Status

Prototype 3.5 — the reformed two-type track model, data-derived segment properties, tyre-grip and downforce physics, and rebuilt option tables with an explicit braking phase — is essentially complete. Current work is validating the optimizer's chosen strategy against Antonelli's real qualifying lap via the exported speed trace, which is where remaining discrepancies in the option-generation physics surface. See `CLAUDE.md` for the detailed, day-to-day progress log.

## Known limitations

- **Race mode is single-lap.** The DP supports race-mode parameters (the race harvest cap, arbitrary starting/ending battery), but doesn't yet loop over multiple laps with state carrying over — that's the next prototype.
- **Curvature is fixed per acceleration/braking calculation** rather than updated live along the segment, because the optimizer's distance stepping doesn't line up with the curvature array's point spacing.
- **Tyre grip is capped at a flat 6g.** Real tyre friction desaturates as normal load rises, which this model doesn't capture — without the cap, the downforce term lets braking deceleration climb past 10g at top speed.
- **Lap-boundary speeds are hardcoded.** The segment list both starts and ends on a `Straight` (Hamilton Straight and T18), so at the wraparound there's no adjacent `Corner` to read a real entry/exit speed from; a fallback constant is used until it's replaced with the real telemetry values.
- **Corners can't be taken deliberately slowly** to harvest more energy — the QSS-derived speed profile is treated as the line taken, rather than one option among a range the optimizer could trade lap time against energy for.
