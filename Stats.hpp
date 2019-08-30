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
    
    // for all
    vd meanDwellTime;
    vd totalDelay;
    vd arrivalHeadwayCv;
    
    // for normal case
    vd bunchingRMSE;
    vd entryDelay;
    vd exitDelay;
    vd paxNo;
    vd entryHeadwayCv;
    
    // for corr case
    vd cvDwellTime;
    vd arrivalHeadwayMean;
    
    vd departHeadwayMean;
    vd departHeadwayCv;
    
    // constructor
    Stats(int objective, int stopSize);
    
    // method:
    // update stats
    void updateNormal(vd &stopDelays, vd &meanDwellTimes, vd &stopEntryDelays, vd &stopExitDelays, vd &stopPaxNos, vd &arrivalHeadwayCvs, vd &stopEntryCv);
    void updateCorr(vd &meanDwellTimes, vd &cvDwellTimes, vd &arrivalHeadwayMeans, vd &arrivalHeadwayCvs, vd &departHeadwayMeans, vd &departHeadwayCvs, vd &stopDelays);
    // unit conversion
    void convertUnit(int totalRuns, int objective);
    
    // print to python
    void printToPython(int argc, char * argv[], int objective);
    
    
};

#endif /* Stats_hpp */
