# This file is responsible for plotting the graph between the optimizer's final chosen battery
# deployment strategy, and Kimi's actual lap. First, we need to run write_speed_trace_csv()
# (called from main.cpp) to produce battery_deployment_silverstone.csv. Then running this file
# will give the plot of the most up to date result.

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
battery_dir = os.path.join(track_data_dir, 'battery_deployment_silverstone.csv')

battery_deployment = pd.read_csv(battery_dir)

# battery_deployment['Distance'] is built by reconstructing the DP's winning path from the
# lap start (Corner segments use their own QSS-derived distance, Straight segments are
# replayed physics accumulated from there), so it's on the same absolute-lap-distance basis
# as telemetry['Distance'] in principle -- but each line is still plotted against its own
# Distance column, both sharing the same axes, in case they don't line up point-for-point.
plt.plot(telemetry['Distance'], telemetry['Speed'], label='Real (FastF1)')
plt.plot(battery_deployment['Distance'], battery_deployment['Speed'], label='Optimizer deployment strategy')
plt.xlabel('Distance (m)')
plt.ylabel('Speed (km/h)')
plt.legend()
plt.savefig('speed_trace_comparison.png')
plt.show()
