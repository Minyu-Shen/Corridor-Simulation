//
//  Stats.hpp
//  Corridor
//
//  Created by Samuel on 5/30/19.
//  Copyright © 2019 samuel. All rights reserved.
//

#ifndef Stats_hpp
#define Stats_hpp
#include <vector>
#include "arena.hpp"

class Stats{
public:

    int stopSize;
    
    // delays at each stop
    std::vector<double> totalDelay;
    std::vector<double> meanService;
    std::vector<double> bunchingRMSE;
    std::vector<double> entryDelay;
    std::vector<double> exitDelay;
    std::vector<double> paxNo;
    
    // constructor
    Stats(int objective, int stopSize);
    
    
    // method:
    // update stats
    void updateNormal(vd &stopDelays, vd &stopServices, vd &stopEntryDelays, vd &stopExitDelays, vd &stopPaxNos, vd &stopBunchingRMSE);
    
    // unit conversion
    void convertUnitNormal(int totalRuns);
    
    // print to python
    void printToPython(int argc, char * argv[]);
    
};

#endif /* Stats_hpp */
