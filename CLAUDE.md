# Project: F1 Battery Deployment Optimizer

## Goal
A C++ project simulating and optimizing battery (MGU-K) deployment across a lap,
targeting 2026 F1 regulations. Built to strengthen my CV for Red Bull (CFD Software
Engineering) and Alpine (Software Engineering placement) applications. We are only focusing on the silverstone race track for this software for now. If there is time more tracks can be added in the future. Isn't a priority for now

## Domain context — 2026 F1 power unit regulations
- Battery: has a maximum charge of 4MJ usable energy cap at any time. No MGU-H — MGU-K only (simpler single source/sink energy balance vs. the old car).
- MGU-K: Maximum rate of deploy and recover capped at 350kW (up from 120kW previously).
- Deployment: up to 4MJ bursts per deployment(~11.5s at full power), multiple deployments per lap allowed if battery supports it (old rules allowed one deployment window/lap). Deployment tapers above 290km/h, hits zero at 355km/h (leading car). A following car within 1s gets MOM, which allows full 350kW up to 337km/h.
- Harvest per lap (Silverstone, 2026 British GP): regulated per-session cap,
  not a physical estimate — qualifying ≈ 6.5MJ, race ≈ 8.0MJ. Qualifying is the
  circuit-sensitive number (FIA cuts it per-event when a track doesn't offer
  enough natural braking energy); race stays close to a flat ~8MJ baseline
  across circuits. Model these as two separate input parameters
  (qualifying_harvest_cap_MJ / race_harvest_cap_MJ), not one shared constant.
- Harvest methods: braking, partial throttle, coasting, "superclipping"
  (diverting engine power to battery at full throttle). Cap: 350kW as of the
  Miami GP mid-season update (was 250kW at season start).
- Harvesting value assumptions(unverifiable estimates): heavy breaking ≈ 250-350kW, coasting ≈ 50-200kW, partial throttle ≈ 50-200kW, superclipping ≈ 250-350kW.
- 1hp = 0.7457kW


## Target circuit: Silverstone
Chosen deliberately — few heavy braking zones (long, fast, flowing corners like
Maggotts-Becketts-Chapel) means harvesting opportunities are scarce, making
energy management a genuinely tight, interesting problem rather than a simple
"brake a lot, deploy on straights" model.

## Core approach
- Lap modeled as a sequence of discrete track segments (straight/corner/braking),
  each with distance and estimated energy demand/harvest potential.
- Battery state modeled with the real constraints above.
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
- **Days 1-9 (current phase): core working prototype.**
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
- Testing: not yet decided — Catch2 or Google Test
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
- [ ] Project skeleton (CMake + Ninja building)
- [ ] Segment data model + unit tests
- [ ] Battery state model
- [ ] DP optimizer core, consider how to optimize recharge, how much hp goes to superclipping etc..
- [ ] Qualifying mode
- [ ] Race mode (multi-lap)

## MVP (ie the core prototype after 9 days)
a CLI that takes no arguments (or minimal ones), builds the Silverstone segment list internally, runs the DP(or whatever optimization algorithm I make) for qualifying mode and race mode, and prints something like a per-segment table showing battery level and deployment decision, the change in delta when we decide to deploy/recharge at each segment, and total lap time — ideally alongside a naive baseline that we can compare to (e.g. "always deploy fully when possible", "never deploys", or "deploys the same amount in every segment") so the DP's improvement is visible and quantifiable, and we can compare the overall improvement in delta as well. More specific definitions and specifications can be found in PROJECT_SPEC.md.