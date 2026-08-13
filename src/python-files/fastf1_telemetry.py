import fastf1
import os
cache_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'cache')
os.makedirs(cache_dir, exist_ok=True)
fastf1.Cache.enable_cache(cache_dir)

session = fastf1.get_session(2026, "British Grand Prix", "Q")
session.load()

lap = session.laps.pick_drivers("ANT").pick_fastest()

telemetry  = lap.get_telemetry()

ant_track_data_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'data', 'track-data', 'silverstone_antonelli_quali.csv')
telemetry[['Distance', 'X', 'Y']].to_csv(ant_track_data_dir, index=False)


print(telemetry[['Time', 'Distance', 'X', 'Y']])