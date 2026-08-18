MVP Definition (ie the core prototype after 9 days):
Track = list of segments
Each segment can be in three category: straight, slow corner, and fast corner.
Common attributes: length, name, type, time
slow corner/fast corner unique attributes: minimum apex speed, exit speed, and throttle %
Car has speed + battery state

Final product:
A CLI that takes no arguments (or minimal ones), builds the Silverstone segment list internally, runs the DP(or whatever optimization algorithm I make) for qualifying mode and race mode, and prints something like a per-segment table showing battery level and deployment decision, the change in delta when we decide to deploy/recharge at each segment, and total lap time — ideally alongside a naive baseline that we can compare to (e.g. "always deploy fully when possible", "never deploys", or "deploys the same amount in every segment") so the DP's improvement is visible and quantifiable, and we can compare the overall improvement in delta as well. 


1. Segment — the static track description
This is pure data: what does this piece of track look like, physically? Fields like segment name/id, type (straight/corner/braking-zone), distance, and some representation of energy potential (however you choose to model it — could be a single "harvest potential" value, or separate demand/recovery values, that's your design decision). This layer knows nothing about strategy — it's just "here's what Silverstone looks like," fixed regardless of what car or driver runs it.
2. BatteryState — the dynamic, changing part
This tracks how much energy is currently stored (in MJ), and enforces the rules: the 4MJ cap, the 350kW deploy/recover rate limit (which really means: how much energy can physically move in/out during one segment, given its duration), and the track's per-lap harvest cap. This layer is where the actual F1 regulation numbers live — it's the part of your code you'd point to and say "this enforces the real 2026 rules."
3. The optimizer — where Segment sequence meets BatteryState over time
This is your DP: given the fixed sequence of segments (the track) and the battery rules (the constraints), find the deployment/harvest decision at each segment that minimizes total lap time, for a given starting battery level. The "state" in your DP is (which segment you're at, how much battery you have) — and you're computing, for every possible battery level at every segment, what the best achievable remaining lap time is.

Segment class: Version 1 we can assume linear affect of deployment/harvesting on time/delta. A limitation for now.Future versions we can make this more accurate including tapers, braking harvesting which doesn't affect delta, etc.

Segment class stay immutable. It is a container of track information, which will be used by the optimization algorithm. 

We can break down any track into X different cases: straights, slow-corners, fast-corners:
- For a segment of track with low-speed corners and heavy breaking zones. The speed of the car that goes through that section is independent of the battery state, it is limited by the grip of the car. Therefore, no need to worry about the deployment in these section. Rather just calculate the amount of battery harvested from these sections. For these sections, the main way to recharge is braking, and partial throttle.
- For a long straight after a corner. Grip isn't the limiting factor anymore, the entry speed to the straight would be the same as the exit speed of the corner. This is where we need to worry about deployment and harvesting. The obvious choice here is to deploy at the start of the straight, then harvest at the end of the straight. When, and how much to deploy/harvest at the start/end of the straight will be a choice to make. The methods to recharge here is through superclipping, as the driver need to full throttle here, but how much to superclip is a decision to make. 
- For other types of corners, like high speed corners, the main harvesting methods are partial throttle and lift-and-coast. Then the amount that we could harvest here would depend on how much the driver lifts and how much of the throttle the driver doesn't need. Throttle percentages might be a thing to consider as well. 

Definition of straight: places where the driver can keep at 100% throttle. However this can vary lap by lap. So for simplification sake, we will pick a point at the exit of a corner where the driver will most likely to be 100% on throttle, without worrying about grip.

Choices of how to harvest on straights: For now, we assume the driver deploys, then harvests. We just need to determine how long to deploy for, and a % on how much to deploy, and a % on how much to harvest. In the future we can make this model more complex. For example, the deployment/harvesting over a straight can be a 2D graph with x-axis as time, and y-axis as % on deployment/harvest. Harvesting will have positive % and deployment will have negative %. And this can vary every millisecond. Also, my DP optimizer cannot work on continous data. Therefore this simplification is needed.

Definition of corners: places where the driver cannot keep at 100% throttle. 

Choices of how to harvest in corners: For now, we harvest everything in braking, no deployment needed because grip is the limiting factor. When lifting and coasting, all power from engine goes to harvesting. When partial throttle, the % of throttle not used from the engine will go to harvesting. However, that will be too complex for the prototype, so for now we won't record the throttle position. Instead, we use the difference between entry speed and apex minimum speed to calculate how much to slow down.


Future ideas that can be implemented: (Recorded here just in case forget):
- Giving battery deployment plan to best attack for a position
- Giving battery deployment plan to defend for a position
- Include tapering function, so far we still haven't considered that. And if we do, we would need to use numerical methods to estimate energy deployment instead of using physics formulas.
- Multi-threading, or any performance improvement optimization techniques
- Bring in read track data (Not sure what to use them for just yet)
- Make the car model more realistic, bring in effects of tire wear, aerodynamics when trailing a car, straight line mode which reduces drag, engine RPM affects battery recharging, etc. (If thought of more write them here)
- Maybe extend this to incorporate with C#, with a strategy software.
- More accurate lap model. 
- Use docker to make it deployable
- A GUI for easy visualization
- Allow simulation for other tracks, not just Silverstone (Maybe even predict pole lap speed for future races)
- 

## Design decision: straight-mode vs corner-mode drag, and only 3 tapering lookup tables (not 4)

Following on from the "straight line mode which reduces drag" idea above: the drag-involved physics formulas were all using one generic drag coefficient, not distinguishing straight-mode (low drag, DRS-open-equivalent) from corner-mode (higher drag). This matters specifically for MOM: reaching MOM's full-350kW threshold (337km/h) needs the lower straight-mode drag to be achievable at all within realistic power figures.

Decision: build 3 tapering lookup tables, not the full 2x2 combination of {MOM, no-MOM} x {straight-mode, corner-mode}:
- no-MOM + corner-mode
- no-MOM + straight-mode
- MOM + straight-mode

Deliberately skipping MOM + corner-mode. This is **a design choice based on real circuit geometry, not a claim that it's physically impossible**: no real circuit has a corner long enough to sustain full-power acceleration up to 337km/h while still being classified as corner-mode. Any stretch of track long enough to do that would, by definition, be mapped as straight-mode instead — reducing drag and saving battery on long/near-straight sections is the entire point of straight-mode in the first place. So MOM+corner-mode is a combination that shouldn't come up given how the track itself gets classified, not a state the physics forbids.

## Making the calculation of drag coefficient a seperate function
This decision was made to allow future drag calculation to be easily integrated with more complexity. E.g. The function can bring in more parameters to consider like the distance that is trailing the car ahead, it will decrease the drag. This can be calculated seperately without affecting the functions that considers drag. Making the code more modular and maintainable. 

## Replacing manual track classification with a quasi-steady-state (QSS) speed profile

### Motivation
This decision was made because the original track model relied on manually classifying each segment as a Straight, FastCorner, or SlowCorner, with apex/entry/exit speeds estimated by eye from a YouTube onboard lap. This had two problems: the speed values were unverified guesses rather than grounded in real track geometry, and every new circuit would require repeating the same manual, error-prone process from scratch.

This direction came from a conversation with the Applied Racing Dynamics (ARD) team, who described how real lap-time simulation is built from fine-grained curvature data rather than hand-picked segments — and who reasonably questioned why my speed values weren't grounded in anything measurable.

### What changed
Track geometry now comes from real position telemetry (`X`, `Y`, `Distance`) pulled via FastF1 for the actual 2026 British Grand Prix. Curvature at each point is derived from this position data using a discrete three-point curvature estimate — this derivation is my own, not something FastF1 provides.

From curvature, I compute a quasi-steady-state (QSS) speed profile:
- A grip-limited speed bound at every point via the friction-circle approximation, `v_limit = sqrt(mu * g / kappa)`.
- A forward pass enforcing the car's maximum acceleration, capped at `v_limit`.
- A backward pass enforcing maximum braking deceleration, also capped at `v_limit`.
- The pointwise minimum of both passes gives the achievable baseline speed profile for the lap.

This is a standard technique in lap-time simulation — it does not require full nonlinear optimal control to compute, only geometry and the car's straight-line acceleration/braking limits, which I already had from the energy-deployment physics.

### Why this is an improvement, specifically
- **Correctness is now checkable against ground truth**: 
  the QSS profile can be plotted directly against FastF1's real recorded speed trace for the same lap, giving a genuine validation metric (e.g. RMS speed error) rather than relying on estimates with no way to confirm accuracy.
- **Segment classification becomes derived, not manual**:
  braking zones, straights, and corner apexes now   fall out of the profile's shape automatically (decreasing sections are braking zones, local minima are apexes) rather than being assigned by eye.
- **Generalizes to any circuit with available telemetry**: 
  adding a new track no longer requires manually re-deriving segment data — only pulling its telemetry and re-running the same curvature/QSS pipeline.

### What did not change
The DP optimizer, battery-state model, and energy-deployment physics are unaffected — QSS replaces how segment reference values (apex speed, entry/exit speed, segment boundaries) are generated, not how they're consumed. The straight/slow-corner/fast-corner option-generator interfaces remain the same.


## Picking values for tyre friction coefficient and downforce coefficient

The QSS model initially used placeholder constants `FRICTION_COEFF = 1.7`and `DOWNFORCE_COEFF = 1.9`. Plotting the resulting speed profile against Antonelli's real 2026 British GP qualifying lap (pulled via FastF1) showed the simulation braking far harder than the real car at every corner — both slow and fast corners were affected, not just one class, which pointed at the grip model itself being under-calibrated rather than a track-geometry or curvature bug.

To calibrate, I picked six corner apexes from the real telemetry and read off each one's real speed (FastF1's `Speed` channel) alongside the curvature already computed at that same point by the QSS pipeline's own `compute_curvature`. A seventh candidate, Turn 3, was excluded: the real lap's minimum-speed point there sits noticeably further down the track than the simulation's minimum-speed point, meaning the two weren't measuring the same physical location and would have contaminated the fit.

Sector    Kimi's speed/kmh   simulation speed/kmh   curvature
Turn 4          87                  59.7985         0.05208
Copse           282                 156.259         0.0121939
Turn 13         221                 94.209          0.027694
Stowe           239                 107.454         0.0220607
Turn 16         102                 83.8747         0.0340649
Turn 17         130                 67.0154         0.0514673

(Simulation speed isn't used in the calibration itself — it's included to show how far off the uncalibrated model was.) These six points were chosen specifically because they're apexes: the point of minimum speed through a real corner is where the driver is fully committed and lateral grip is maxed out, which is exactly the condition the QSS force-balance equation assumes — `v²k = µg + (µk_z/m)v²`, required lateral force equal to available lateral force.

Treating `x = v²` and `y = v²k`, that equation is linear: `y = Bx + A`, with `B = µk_z/m` and `A = µg`. Fitting a least-squares line through the six `(x, y)` points and solving back for the two constants gave y-intercept as 37.0955, and gradient of 0.0105. Which results in `µ ≈ 3.78` and a downforce constant equivalent to `DOWNFORCE_COEFF ≈ 2.40`.

I checked these against commonly-cited real-world F1 lateral-g and downforce figures, and there are no reliable sources for the 2026 regulations. We could deduce the figures through 2026 regulation aiming to cut downforce by roughly 25%-30% relative to 2022–2025-spec cars, but that would mean adding another layer of guesswork on top of an already-approximate source. Therefore, calibrating directly against this lap's own real telemetry is more directly grounded than adjusting a secondary-source figure for a spec with no public data yet, so I kept the fitted values rather than literature-adjusted ones.

The fit itself is loose. After plotting the graph it results in a correlation of 0.7344, since a two-parameter fit over only six points can't fully capture real corner-to-corner variation in effective grip (tyre condition, driver commitment, aero effects the model doesn't capture). However, it's still a large improvement compared to the original placeholder constants, and is good enough for this project's purposes.