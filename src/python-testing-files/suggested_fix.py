import numpy as np

def time_to_reach_velocity_numeric(v_target, v0, m, P, k, dt=1e-5):
    v = v0
    t = 0.0
    # decelerating case (v0 > v_target)
    if v0 > v_target:
        while v > v_target:
            dvdt = (P / v - k * v**2) / m
            v += dvdt * dt
            t += dt
    else:
        while v < v_target:
            dvdt = (P / v - k * v**2) / m
            v += dvdt * dt
            t += dt
    return t


v_t = float(input("Enter target speed in km/h"))
v0 = float(input("Enter initial speed in km/h"))
m = 768
P = float(input("Enter power in Watts"))
k = 0.5 * 0.8 * 1.225

print("time is :",time_to_reach_velocity_numeric(v_t, v0, m, P, k))
