#pragma once
#include "car.h"
#include "track.h"
#include "physics.h"

class Optimizer{

    private:
        bool race_mode;
        bool mom;
        Track circuit = Track();
        Car car = Car(race_mode, mom);

    public:
        Optimizer(bool race_mode, bool mom);

        double optimize();
        double estimate_deploy_distance();
        
};