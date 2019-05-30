//
//  Stats.cpp
//  Corridor
//
//  Created by Samuel on 5/30/19.
//  Copyright © 2019 samuel. All rights reserved.
//

#include "Stats.hpp"

Stats::Stats(int objective, int stopSize){
    if (objective == 0) { // normal case
        totalDelay = std::vector<double> (stopSize+1);
        meanService = std::vector<double> (stopSize);
        bunchingRMSE = std::vector<double> (stopSize);
        entryDelay = std::vector<double> (stopSize);
        exitDelay = std::vector<double> (stopSize);
        paxNo = std::vector<double> (stopSize);
    }
    
    
}
