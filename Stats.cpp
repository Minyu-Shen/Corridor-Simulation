//
//  Stats.cpp
//  Corridor
//
//  Created by Samuel on 5/30/19.
//  Copyright © 2019 samuel. All rights reserved.
//

#include "Stats.hpp"
#include <iostream>

Stats::Stats(int objective, int stpSize){
    stopSize = stpSize;
    totalDelay = vd (stopSize+1);
    arrivalHeadwayCv = vd (stopSize);
    entryHeadwayCv = vd (stopSize);
    
    meanDwellTime = vd (stopSize);
    if (objective == 0) { // normal case
//        bunchingRMSE = vd (stopSize);
        entryDelay = vd (stopSize);
        exitDelay = vd (stopSize);
        paxNo = vd (stopSize);
    } else {
        cvDwellTime = vd (stopSize);
        arrivalHeadwayMean = vd (stopSize);
        departHeadwayMean = vd (stopSize);
        departHeadwayCv = vd (stopSize);
    }
}

void Stats::updateNormal(vd &stopDelays, vd &meanDwellTimes, vd &stopEntryDelays, vd &stopExitDelays, vd &stopPaxNos, vd &arrivalHeadwayCvs, vd &stopEntryCv){
    addVector(totalDelay, stopDelays);
    addVector(meanDwellTime, meanDwellTimes);
    addVector(entryDelay, stopEntryDelays);
    addVector(exitDelay, stopExitDelays);
    addVector(paxNo, stopPaxNos);
    addVector(arrivalHeadwayCv, arrivalHeadwayCvs);
    addVector(entryHeadwayCv, stopEntryCv);
}

void Stats::updateCorr(vd &meanDwellTimes, vd &cvDwellTimes, vd &arrivalHeadwayMeans, vd &arrivalHeadwayCvs, vd &departHeadwayMeans, vd &departHeadwayCvs, vd &stopDelays){
    addVector(meanDwellTime, meanDwellTimes);
    addVector(cvDwellTime, cvDwellTimes);
    addVector(arrivalHeadwayMean, arrivalHeadwayMeans);
    addVector(arrivalHeadwayCv, arrivalHeadwayCvs);
    addVector(departHeadwayMean, departHeadwayMeans);
    addVector(departHeadwayCv, departHeadwayCvs);
    addVector(totalDelay, stopDelays);
}

void Stats::convertUnit(int totalRuns, int objective){
    if (objective == 0) {
        multiplyVector(totalDelay, 1.0/60.0/totalRuns);
        multiplyVector(meanDwellTime, 1.0/60.0/totalRuns);
//        multiplyVector(bunchingRMSE, 1.0/60.0/totalRuns);
        multiplyVector(arrivalHeadwayCv, 1.0/totalRuns);
        multiplyVector(entryHeadwayCv, 1.0/totalRuns);
        multiplyVector(entryDelay, 1.0/60.0/totalRuns);
        multiplyVector(exitDelay, 1.0/60.0/totalRuns);
        multiplyVector(paxNo, 1.0/totalRuns);
    }else{
        multiplyVector(meanDwellTime, 1.0/60.0/totalRuns);
        multiplyVector(cvDwellTime, 1.0/totalRuns);
        multiplyVector(arrivalHeadwayMean, 1.0/60.0/totalRuns);
        multiplyVector(arrivalHeadwayCv, 1.0/totalRuns);
        multiplyVector(departHeadwayMean, 1.0/60.0/totalRuns);
        multiplyVector(departHeadwayCv, 1.0/totalRuns);
        multiplyVector(totalDelay, 1.0/60.0/totalRuns);
    }
}

void Stats::printToPython(int argc, char * argv[], int objective){
    // shout out to the python console and log file
    if (objective == 0) {
        double delayCumSum = 0.0; double pNo = 0.0;
        double entryDelayCumSum = 0.0; double exitDelayCumSum = 0.0;
        double servTimeCumSum = 0.0; double arrivalVariation = 0.0;
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
//                arrivalVariation = bunchingRMSE[s-1];
                /****************** change here to different cv output (entry or arrival) ******************/
//                arrivalVariation =arrivalHeadwayCv[s-1];
                arrivalVariation = entryHeadwayCv[s-1];
                pNo = paxNo[s-1];
            }
            
            if (s != 0) {
                servTimeCumSum += meanDwellTime[s-1];
            }
            std::cout << entryDelayCumSum << " " << exitDelayCumSum << " " << delayCumSum << " " << servTimeCumSum << " " << arrivalVariation << " " <<  pNo << std::endl;
        }
    } else { // correlation case
        
        for (int s = 0; s < stopSize+1; s++) {
            for(int i = 1; i < argc; ++i) std::cout << argv[i] << " ";
            std::cout << s << " ";
            if (s != 0) {
                std::cout << meanDwellTime[s-1] << " " << cvDwellTime[s-1] << " " << arrivalHeadwayMean[s-1] << " " << arrivalHeadwayCv[s-1] << " " << departHeadwayMean[s-1] << " " <<  departHeadwayCv[s-1] << " " << totalDelay[s-1] << std::endl;
            }else{
                std::cout << 0.0 << " " << 0.0 << " " << 0.0 << " " << 0.0 << " " << 0.0 << " " <<  0.0 << " " <<  0.0 << std::endl; // for now, all the consolidation indicators are set to be 0.0
            }
        }
    }
        
        
    
}
