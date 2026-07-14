import math

mass = 768
def ke(speed):
    speed = speed/3.6
    energy = 0.5 * speed * speed * mass
    return energy

def work_done_with_drag(r, vi, x):
    r = r * 1000
    vi = vi / 3.6
    CdA = 0.8
    rou = 1.225
    k = 0.5 * rou * CdA
    energy = mass/2 *((r/k - (r/k - vi**3)*math.e**(-3*k*x/mass))**(2/3) - vi**2)

    return energy


def power_output(vi, w, x):
    vi = vi / 3.6
    CdA = 0.8
    rou = 1.225
    k = 0.5 * rou * CdA

    r = ((vi**2 + 2*w/mass)**1.5 - vi**3 * math.e**(-3*k*x/mass))*k/(1- math.e**(-3*k*x/mass))

    return r

def lift_off(vi, x):
    vi = vi / 3.6
    CdA = 0.8
    rou = 1.225
    k = 0.5 * rou * CdA    

    
    energy = (mass * vi**2 / 2) * (1-math.e**(-2*k*x/mass))

    return energy

def dis(vi,w,r):
    vi = vi / 3.6
    CdA = 0.8
    rou = 1.225
    k = 0.5 * rou * CdA
    r = r*1000

    x = (-mass/3*k) * math.log((r/k - (vi**2 + 2*w/mass)**1.5) / (r/k - vi**3))

    return x

txt = ""


while(txt != "q"):
    print("-------------------------------------------------")
    print("I have a few you can use:")
    print("Type 1 for calculation of kinetic energy")
    print("Type 2 for calculation of energy with respect to drag")
    print("Type 3 for energy diff between two speed")
    print("Type 4 for calculating required power output from engine")
    print("Type 5 for calculating energy taken away by drag when lifting off")
    print("Type 6 for calculating distance needed to recharge the amount of energy")
    print("Type q to quit")
    txt = input("what do you need?  ")
    
    if(txt == "1"):
        speed = float(input("Enter speed in km/h: "))
        print("Energy:",round(ke(speed)))
        
    elif(txt == "2"):
        vi = float(input("Enter initial speed in km/h: "))
        r = float(input("Enter power output in kW: "))
        x = float(input("Enter distance to travel:"))
        print("Energy:",round(work_done_with_drag(r, vi, x)))
    elif(txt=="3"):
        speed1 = float(input("Enter speed in km/h: "))
        speed2 = float(input("Enter speed in km/h: "))
        print("Energy:",round(ke(speed1) - ke(speed2)))
    elif(txt=="4"):
        vi = float(input("Enter speed in km/h: "))
        w = float(input("Enter work done in Joules: "))
        x = float(input("Enter distance: "))
        print("Power output:",round(power_output(vi, w, x)))
    elif(txt=="5"):
        vi = float(input("Enter speed in km/h: "))
        x = float(input("Enter distance: "))
        print("Energy:",round(lift_off(vi, x)))
    elif(txt=="6"):
        vi = float(input("Enter speed in km/h: "))
        w = float(input("Enter work done in Joules: "))
        r = float(input("Enter power output in kW: "))
        print("Distance:",round(dis(vi,w,r)))

    else:
        print("invalid input")
        
