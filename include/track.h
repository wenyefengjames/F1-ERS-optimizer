#pragma once
#include "../../include/fast-corner.h"
#include "../../include/slow-corner.h"
#include "../../include/straight.h"
#include <memory>
#include <vector>

class Track{

    private:
        std::vector<std::unique_ptr<Segment>> track;

    public:

        Track();


};