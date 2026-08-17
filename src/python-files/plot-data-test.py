import pandas as pd
import matplotlib.pyplot as plt
import os
import fastf1
cache_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'cache')
os.makedirs(cache_dir, exist_ok=True)
fastf1.Cache.enable_cache(cache_dir)

session = fastf1.get_session(2026, "British Grand Prix", "Q")
session.load()

lap = session.laps.pick_drivers("ANT").pick_fastest()

telemetry = lap.get_telemetry()

track_data_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'data', 'track-data')
qss_dir = track_data_dir = os.path.join(track_data_dir, 'qss_antonelli.csv')

qss = pd.read_csv(qss_dir)

plt.plot(telemetry['Distance'], telemetry['Speed'], label='Real (FastF1)')
plt.plot(telemetry['Distance'], qss['Speed'], label='QSS model')
plt.xlabel('Distance (m)')
plt.ylabel('Speed (km/h)')
plt.legend()
plt.savefig('speed_trace_comparison.png')