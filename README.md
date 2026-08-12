# F1 Battery Deployment Optimizer

A C++ simulator and optimizer for MGU-K battery deployment strategy across a Formula 1 lap, built against the 2026 F1 power unit regulations. Currently modeled for Silverstone only.

## Why this project exists

Under the 2026 regulations, I spotted that battery management throughout a lap becomes genuinely critical — not just for outright lap time, but for defending position against a car attacking from behind, or for picking the right moment to attack yourself. I built this project to explore and solve that problem directly.

Silverstone was chosen deliberately as the target circuit: its long, fast, flowing corners (Maggotts–Becketts–Chapel) leave very few heavy braking zones, so energy-harvesting opportunities are genuinely scarce. That scarcity is what makes energy management here a real optimization problem, rather than a trivial "brake hard, deploy on the straights" strategy.

## What it does

Given a lap — modeled as a sequence of straights, fast corners, and slow corners — the optimizer uses dynamic programming to decide *when* and *how much* battery energy to deploy or harvest at every point on track, minimizing total lap time under the real constraints of the regulations:

- 4MJ maximum usable battery charge at any time
- MGU-K deployment/recovery rate capped at 350kW
- Deployment power tapers linearly above 290km/h (337km/h with MOM / Overtake Mode active), reaching zero at 355km/h
- Per-session harvest caps (qualifying vs. race), with a +0.5MJ bonus when MOM is active in race mode
- "Straight Mode" aerodynamics reducing drag on specific straights, with their own start/end windows

It supports both qualifying (single-lap, no carry-over) and race (harvest state tracked against the per-lap cap) modes, with MOM handled as a separate, gap-to-car-ahead-dependent input.

## Documentation

This README covers the what and how. Two other files cover the rest:
- **`CLAUDE.md`** — the detailed, day-to-day progress log: what's actually built vs. planned, known bugs and limitations, and the prototype-by-prototype roadmap.
- **`PROJECT_SPEC.md`** — the reasoning behind specific design choices (why particular tradeoffs were made where more than one approach was reasonable).

## Architecture

- **`Segment`** (base class) / **`Straight`**, **`FastCorner`**, **`SlowCorner`** (derived) — the track data model. Each segment carries its own length, entry/apex/exit speed constraints, and (for straights) Straight Mode window.
- **`Track`** — the ordered sequence of segments making up a lap (currently hardcoded to Silverstone), with wraparound `next()`/`prev()` traversal.
- **`Battery`** — owns charge state, the per-lap harvest limit, and the qualifying/race/MOM mode switch.
- **`Car`** — physical constants (ICE/MGU-K power, mass), holds a `Battery`.
- **`physics`** namespace — stateless drag/kinetic-energy/taper formulas. The drag ODEs are solved numerically (Euler integration) since a speed-dependent taper curve has no clean closed-form integral; a precomputed lookup table (`taper_table` / `search_taper_table`) avoids re-deriving the same taper trajectory on every call.
- **`Optimizer`** — the DP core. Backward induction over a discretized state space of (segment, battery level, target ending battery, harvest so far), memoized into a flattened table. Each segment's set of feasible (deploy, harvest, Δtime) options is precomputed once per segment at full battery and cached, rather than recomputed at every distinct battery level the DP visits — the DP's own feasibility check (`Battery::check_allow_charge`) correctly filters out any cached option that doesn't actually fit at a given state, so nothing is lost by computing at the maximum once.

## Project layout

```
include/        Public headers for every class above
src/            Implementations, plus src/track-model/ for the segment classes
test/           GoogleTest suite
benchmark/      Google Benchmark suite
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

## Testing and tooling

- **GoogleTest** — hand-computed exact-value tests for simple/linear formulas, and property-based tests (boundary values, monotonicity, round-trip invariants) for the transcendental drag/taper ODEs, where a hand-typed expected value would just re-derive the implementation rather than check it.
- **Google Benchmark** — used to validate that performance work (the segment-option-table caching, the minimum-deployment-power search step size) actually helps, rather than assuming it does.
- **clang-tidy** — configured for `bugprone-*`, `performance-*`, `modernize-*`, and `clang-analyzer-*` checks (see `.clang-tidy`).
- **AddressSanitizer / UndefinedBehaviorSanitizer** — via the `ENABLE_SANITIZERS` CMake option above.

## Status

Prototype 2 — a refined track representation, numerically-accurate taper/drag physics, and MOM-aware harvest limits — is complete. Prototype 3 work (performance, a more general/data-driven track model, refined physics, visualization) is in progress. See `CLAUDE.md` for the detailed, day-to-day progress log.

## Known limitations

- Fast corners treat `apex_min_speed` as a hard ceiling rather than a range the optimizer can explore — taking a corner slower than necessary in exchange for harvesting more energy isn't currently modeled.
- Slow corners still brake to a fixed entry speed rather than the optimizer choosing where to brake for the best energy/time tradeoff.
