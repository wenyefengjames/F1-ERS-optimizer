# This file is responsible for plotting the graph between QSS simulation, and Kimi's actual lap.
# First, we need to run the function of 'write_csv()' from 'track-generation.cpp' to produce the values of
# the simulation results. Then running this file will give the plot of the most up to date result.

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
qss_dir = os.path.join(track_data_dir, 'qss_silverstone.csv')

qss = pd.read_csv(qss_dir)

# qss['Distance'] is on its own scale/reference (chord-summed from the TUMFTM
# Silverstone.csv x_m/y_m data), so it won't line up point-for-point with
# telemetry['Distance'] -- each line is plotted against its own Distance column,
# both sharing the same axes.
plt.plot(telemetry['Distance'], telemetry['Speed'], label='Real (FastF1)')
plt.plot(qss['Distance'], qss['QSS Speed'], label='QSS model')
plt.xlabel('Distance (m)')
plt.ylabel('Speed (km/h)')
plt.legend()
plt.savefig('speed_trace_comparison.png')
plt.show()
