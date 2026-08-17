import fastf1
import os
cache_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'cache')
os.makedirs(cache_dir, exist_ok=True)
fastf1.Cache.enable_cache(cache_dir)

session = fastf1.get_session(2026, "British Grand Prix", "Q")
session.load()

lap = session.laps.pick_drivers("ANT").pick_fastest()

telemetry  = lap.get_telemetry()

# FastF1's X/Y position channels aren't in meters -- empirically measured
# (via total lap distance and |dr/ds| checks) at ~10x real-world scale,
# likely decimeters. Distance is unaffected
POSITION_SCALE = 10
telemetry['X'] = telemetry['X'] / POSITION_SCALE
telemetry['Y'] = telemetry['Y'] / POSITION_SCALE

ant_track_data_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'data', 'track-data', 'silverstone_antonelli_quali.csv')
telemetry[['Distance', 'X', 'Y']].to_csv(ant_track_data_dir, index=False)

ant_track_data_dir_with_speed = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'data', 'track-data', 'silverstone_antonelli_quali_with_speed.csv')
telemetry[['Distance', 'Speed']].to_csv(ant_track_data_dir_with_speed, index=False)