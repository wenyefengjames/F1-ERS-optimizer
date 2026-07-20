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

