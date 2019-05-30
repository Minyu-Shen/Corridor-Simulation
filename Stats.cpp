//
//  Stats.cpp
//  Corridor
//
//  Created by Samuel on 5/30/19.
//  Copyright © 2019 samuel. All rights reserved.
//

#include "Stats.hpp"


Stats::Stats(int objective, int stpSize){
    stopSize = stpSize;
    if (objective == 0) { // normal case
        totalDelay = std::vector<double> (stopSize+1);
        meanService = std::vector<double> (stopSize);
        bunchingRMSE = std::vector<double> (stopSize);
        entryDelay = std::vector<double> (stopSize);
        exitDelay = std::vector<double> (stopSize);
        paxNo = std::vector<double> (stopSize);
    } else {
        
    }
    
}

void Stats::updateNormal(vd &stopDelays, vd &stopServices, vd &stopEntryDelays, vd &stopExitDelays, vd &stopPaxNos, vd &stopBunchingRMSE){
    addVector(totalDelay, stopDelays);
    addVector(meanService, stopServices);
    addVector(entryDelay, stopEntryDelays);
    addVector(exitDelay, stopExitDelays);
    addVector(paxNo, stopPaxNos);
    addVector(bunchingRMSE, stopBunchingRMSE);
}

void Stats::convertUnitNormal(int totalRuns){
    multiplyVector(totalDelay, 1.0/60.0/totalRuns);
    multiplyVector(meanService, 1.0/60.0/totalRuns);
    multiplyVector(bunchingRMSE, 1.0/60.0/totalRuns);
    multiplyVector(entryDelay, 1.0/60.0/totalRuns);
    multiplyVector(exitDelay, 1.0/60.0/totalRuns);
    multiplyVector(paxNo, 1.0/totalRuns);
}

void Stats::printToPython(int argc, char * argv[]){
    // shout out to the python console and log file
    double delayCumSum = 0.0;
    double entryDelayCumSum = 0.0;
    double exitDelayCumSum = 0.0;
    double servTimeCumSum = 0.0;
    double arrivalVariation = 0.0;
    double pNo = 0.0;
    for (int s = 0; s < stopSize+1; s++) {
        for(int i = 1; i < argc; ++i) std::cout << argv[i] << " ";
        std::cout << s << " ";
        
        // last element is the delay in the consolidation stop
        if (s == 0){
            delayCumSum += totalDelay[stopSize];
            arrivalVariation = 0.0;
            pNo = paxNo[0];
        }else{
            delayCumSum += totalDelay[s-1];
            entryDelayCumSum += entryDelay[s-1];
            exitDelayCumSum += exitDelay[s-1];
            arrivalVariation = bunchingRMSE[s-1];
            pNo = paxNo[s-1];
        }
        
        if (s != 0) {
            servTimeCumSum += meanService[s-1];
        }
        std::cout << entryDelayCumSum << " " << exitDelayCumSum << " " << delayCumSum << " " << servTimeCumSum << " " << arrivalVariation << " " <<  pNo << std::endl;
    }
}
