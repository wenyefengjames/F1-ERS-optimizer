import fastf1
import os
os.makedirs('cache', exist_ok=True)
fastf1.Cache.enable_cache('cache')

session = fastf1.get_session(2026, "British Grand Prix", "Q")
session.load()

lap = session.laps.pick_driver("ANT").pick_fastest()

telemetry  = lap.get_telemetry()

print(telemetry['Distance'].max())  
print(telemetry['Speed'].max()) 