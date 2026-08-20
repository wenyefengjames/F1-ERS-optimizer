# This file pulls Kimi's fastest lap telemetry from FastF1 for the 2026 British Grand
# Prix qualifying session (used to warm the local FastF1 cache / for manual inspection).
# Track geometry now comes from the TUMFTM Silverstone.csv data source instead, and
# plot-data-test.py does its own independent live pull for the real-speed comparison
# line, so this script no longer writes anything out.

import fastf1
import os
cache_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'cache')
os.makedirs(cache_dir, exist_ok=True)
fastf1.Cache.enable_cache(cache_dir)

session = fastf1.get_session(2026, "British Grand Prix", "Q")
session.load()

lap = session.laps.pick_drivers("ANT").pick_fastest()

telemetry  = lap.get_telemetry()