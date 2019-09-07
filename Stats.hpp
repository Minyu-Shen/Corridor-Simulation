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
    
    vd meanDwellTime;
    vd totalDelay;
    vd totalDelayStd;
    vd entryDelay;
    vd exitDelay;
    vd paxNo;
    vd cvDwellTime;
    vd entryHeadwayMean;
    vd entryHeadwayCv;
    vd arrivalHeadwayMean;
    vd arrivalHeadwayCv;
    vd departHeadwayMean;
    vd departHeadwayCv;
    
    // constructor
    Stats(int objective, int stopSize);
    
    // method:
    // update stats
    void updateNormal(vd &stopDelays, vd &meanDwellTimes, vd &stopEntryDelays, vd &stopExitDelays, vd &stopPaxNos, vd &stopEntryCv);
    void updateCorr(vd &meanDwellTimes, vd &cvDwellTimes, vd &arrivalHeadwayMeans, vd &arrivalHeadwayCvs, vd &entryHeadwayMeans, vd &entryHeadwayCvs, vd &departHeadwayMeans, vd &departHeadwayCvs, vd &stopDelays, vd &stopDelayStds);
    // unit conversion
    void convertUnit(int totalRuns, int objective);
    
    // print to python
    void printToPython(int argc, char * argv[], int objective);
    
    
};

#endif /* Stats_hpp */
