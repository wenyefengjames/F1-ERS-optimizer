import math

mass = 768
def ke(speed):
    speed = speed/3.6
    energy = 0.5 * speed * speed * mass
    return energy

# This function takes into the account of drag to the energy onto the car.
# This function calculates the increase in kinetic energy,
#(it doesn't include the initial kinetic energy of the car),
# after taken away the energy lost to drag.
# Inputs: r - power output from engine, in kW
#         vi - initial speed, in km/h
#         x - distance travelled with the given power output from engine, in meters
# Output: kinetic energy gained to the car, in Joules

def work_done_with_drag(r, vi, x):
    r = r * 1000
    vi = vi / 3.6
    CdA = 0.8
    rou = 1.225
    k = 0.5 * rou * CdA
    energy = mass/2 *((r/k - (r/k - vi**3)*math.e**(-3*k*x/mass))**(2/3) - vi**2)

    return energy

# This function calculates the required power output from engine, for a given initial speed,
# the increase in kinetic energy needed, and the distance allowed for the car to gain this ke.
# Inputs: w - kinetic energy needed to be gained, in Joules
#         vi - initial speed, in km/h
#         x - distance allowed for the car to travel to gain enough energy, in meters
# Output: Power needed from the engine to gain enough KE within the distance given, in Watts
def power_output(vi, w, x):
    vi = vi / 3.6
    CdA = 0.8
    rou = 1.225
    k = 0.5 * rou * CdA

    r = ((vi**2 + 2*w/mass)**1.5 - vi**3 * math.e**(-3*k*x/mass))*k/(1- math.e**(-3*k*x/mass))

    return r


# This function calculates the amount of KE that would be lost to drag if there is no throttle
# Inputs: vi - initial speed, in km/h
#         x - distance that the car will travel without throttle
# Output: KE lost to drag, in Joules

def lift_off(vi, x):
    vi = vi / 3.6
    CdA = 0.8
    rou = 1.225
    k = 0.5 * rou * CdA    

    
    energy = (mass * vi**2 / 2) * (1-math.e**(-2*k*x/mass))

    return energy

# This function calculates the distance that the car needs to travel in order to recharge to the given
# energy target
# Inputs: vi - initial speed, in km/h
#         w - target energy needed to be recharged into the battery
#         r - power output from the engine
# Output: KE lost to drag, in Joules

def dis(vi,w,r):
    vi = vi / 3.6
    CdA = 0.8
    rou = 1.225
    k = 0.5 * rou * CdA
    r = r*1000

    x = (-mass/3*k) * math.log((r/k - (vi**2 + 2*w/mass)**1.5) / (r/k - vi**3))

    return x


def time_to_reach_velocity(v, vi, r):
    """
    Computes the time t for the car to go from velocity vi to velocity v,
    given constant engine power P and drag constant k (net force F = P/v - k*v^2).

    Derived from: m * dv/dt = P/v - k*v^2
    Closed form uses terminal velocity a = (P/k)^(1/3) and partial fractions
    on m*v / (P - k*v^3).

    Parameters
    ----------
    v   : float - target velocity to solve time for
    v0  : float - initial velocity at t0
    m   : float - mass
    P   : float - constant engine power
    k   : float - drag constant (k = 0.5 * rho * Cd * A)
    t0  : float - initial time (default 0)

    Returns
    -------
    t : float - time at which velocity v is reached
    """
    vi = vi / 3.6
    v = v / 3.6
    CdA = 0.8
    rou = 1.225
    k = 0.5 * rou * CdA
    P = r*1000
    m = mass
    t0=0

    a = (P / k) ** (1.0 / 3.0)  # terminal velocity

    def I(vel):
        # antiderivative: (1/k) * [ -1/(3a) * ln|a - vel|
        #                           + 1/(6a) * ln(vel^2 + a*vel + a^2)
        #                           - 1/(a*sqrt(3)) * arctan((2*vel + a) / (a*sqrt(3))) ]
        term1 = -1.0 / (3.0 * a) * math.log(abs(a - vel))
        term2 = 1.0 / (6.0 * a) * math.log(vel**2 + a * vel + a**2)
        term3 = -1.0 / (a * math.sqrt(3)) * math.atan((2.0 * vel + a) / (a * math.sqrt(3)))
        return (1.0 / k) * (term1 + term2 + term3)

    t = t0 + m * (I(v) - I(vi))
    return t

txt = ""


while(txt != "q"):
    print("-------------------------------------------------")
    print("I have a few you can use:")
    print("Type 1 for calculation of kinetic energy")
    print("Type 2 for calculation of energy with respect to drag")
    print("Type 3 for energy diff between two speed")
    print("Type 4 for calculating required power output from engine")
    print("Type 5 for calculating energy taken away by drag when lifting off")
    print("Type 6 for calculating distance needed to gain a target amount of KE")
    print("Type 7 for calculating time needed to reach a terminal speed for given power output")
    print("Type q to quit")
    txt = input("what do you need?  ")
    
    if(txt == "1"):
        speed = float(input("Enter speed in km/h: "))
        print("Energy:",round(ke(speed), 2))
        
    elif(txt == "2"):
        vi = float(input("Enter initial speed in km/h: "))
        r = float(input("Enter power output in kW: "))
        x = float(input("Enter distance to travel:"))
        print("Energy:",round(work_done_with_drag(r, vi, x), 2))
    elif(txt=="3"):
        speed1 = float(input("Enter speed in km/h: "))
        speed2 = float(input("Enter speed in km/h: "))
        print("Energy:",round(ke(speed1) - ke(speed2), 2))
    elif(txt=="4"):
        vi = float(input("Enter speed in km/h: "))
        w = float(input("Enter work done in Joules: "))
        x = float(input("Enter distance: "))
        print("Power output:",round(power_output(vi, w, x), 2))
    elif(txt=="5"):
        vi = float(input("Enter speed in km/h: "))
        x = float(input("Enter distance: "))
        print("Energy:",round(lift_off(vi, x), 2))
    elif(txt=="6"):
        vi = float(input("Enter speed in km/h: "))
        w = float(input("Enter work done in Joules: "))
        r = float(input("Enter power output in kW: "))
        print("Distance:",round(dis(vi,w,r), 2))

    elif(txt=="7"):
        vi = float(input("Enter initial speed in km/h: "))
        v = float(input("Enter final speed in km/h: "))
        r = float(input("Enter power output in kW: "))
        print("Time:",round(time_to_reach_velocity(v, vi, r), 2))
    else:
        print("invalid input")
        
