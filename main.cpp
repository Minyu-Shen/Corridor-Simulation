//
//  main.cpp
//  Corridor-Simulation
//
//  Created by samuel on 7/7/18.
//  Copyright © 2018 Samuel. All rights reserved.
//

#include <iostream>
#include "SimulationConfig.hpp"
#include "arena.hpp"

int main(int argc, char * argv[]) {
    SimulationConfig config(argc, argv);
    Corridor simulator = SimulationConfig::CorridorBuilder(config);
    
    int stopSize = simulator.busGenerator->kStop;
    double warmDuration = 3600.0 * 1.5;
    double peakDuration = 3600.0 * 5.5;
    //    double warmDuration = 3600.0 * 0.25;
    //    double peakDuration = 3600.0 * 0.25;
    double warmTotalPaxRate = 100.0; // pax/hr
    
//    clock_t begin = clock();
//    clock_t end = clock();
//    double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
//    std::cout << elapsed_secs << std::endl;
    
    /* ---------- test runs ---------- */
    double testRuns = 200; //200
    std::vector<double> stopDelayEach (stopSize+1);
    std::vector<double> stopServiceEach (stopSize);
    std::vector<double> stopBunchingEach (stopSize);
    std::vector<double> stopEntryDelayEach (stopSize);
    std::vector<double> stopExitDelayEach (stopSize);
    std::vector<double> stopPaxNoEach (stopSize);
    
    // key is sample no, vector contains the sampels
    // to determine the needed simulation rounds
    std::map<int, std::vector<double>> estimatingRunsMap;
    for (int r = 0; r < testRuns; r++) {
        simulator.busGenerator->schedule(warmDuration, peakDuration);
        
        /* warmup period */
        simulator.paxDemandBounding(warmTotalPaxRate/3600.0/config.kLine);
        
        for (double time = 0.0; time < warmDuration; time += 1.0) {
            simulator.simulate(time);
        }
        
        /* peak period */
        simulator.paxDemandBounding(1.0e8); // relax the bounding
        for (double time = warmDuration; time < warmDuration+peakDuration; time++) {
            simulator.simulate(time);
        }
        
        /* stop delay and service time collections */
        std::vector<double> stopDelays (stopSize+1);
        std::vector<double> stopServices (stopSize);
        std::vector<double> stopBunchingRMSE (stopSize);
        std::vector<double> stopEntryDelays (stopSize);
        std::vector<double> stopExitDelays (stopSize);
        std::vector<double> stopPaxNos (stopSize);
        
        computeMeanDelay(stopDelays, stopServices, stopEntryDelays, stopExitDelays, stopPaxNos, simulator.busGenerator->peakBusVec);
        addVector(stopDelayEach, stopDelays);
        addVector(stopServiceEach, stopServices);
        addVector(stopEntryDelayEach, stopEntryDelays);
        addVector(stopExitDelayEach, stopExitDelays);
        addVector(stopPaxNoEach, stopPaxNos);
        estimatingRunsMap.insert(std::make_pair(r, stopDelays));
        
        /* bunching RMSE estimations */
        
        calculateBunchingRMSE(stopBunchingRMSE, simulator.busGenerator->peakBusVec, 3600.0 / config.meanHeadway);
        addVector(stopBunchingEach, stopBunchingRMSE);
        
//        computeBunchingRMSE(stopBunchingRMSE, simulator.busGenerator->peakBusVec, std::stod(argv[5])/std::stoi(argv[3]), std::stod(argv[4]), warmDuration*3600);
        
        simulator.reset();
//        writeJsonToFile(simulator.jsObject);
//        std::cout << r << std::endl;
        
    } // loop ends
    
    // estimate num of simulations needed
    int realRuns = computeRuns(estimatingRunsMap) - testRuns;
//    std::cout << "real runs: " << realRuns << std::endl;
//    realRuns = 0;
    /* ---------- real runs ---------- */
    for (int r = 0; r < realRuns; r++) {
        simulator.busGenerator->schedule(warmDuration, peakDuration);
        /* warmup period */
        simulator.paxDemandBounding(warmTotalPaxRate/3600.0/config.kLine);
        for (double time = 0.0; time < warmDuration; time += 1.0) {
            simulator.simulate(time);
        }
        /* peak period */
        simulator.paxDemandBounding(1.0e8); // relax the bounding
        for (double time = warmDuration; time < warmDuration+peakDuration; time++) {
            simulator.simulate(time);
        }
        /* stop and link delay collections */
        std::vector<double> stopDelays (stopSize+1);
        std::vector<double> stopServices (stopSize);
        std::vector<double> stopBunchingRMSE (stopSize);
        std::vector<double> stopEntryDelays (stopSize);
        std::vector<double> stopExitDelays (stopSize);
        std::vector<double> stopPaxNos (stopSize);
        
        //        std::vector<double> stopDelays = simulator.collectDelayAsArray();
        computeMeanDelay(stopDelays, stopServices, stopEntryDelays, stopExitDelays, stopPaxNos, simulator.busGenerator->peakBusVec);
        addVector(stopDelayEach, stopDelays);
        addVector(stopServiceEach, stopServices);
        addVector(stopEntryDelayEach, stopEntryDelays);
        addVector(stopExitDelayEach, stopExitDelays);
        addVector(stopPaxNoEach, stopPaxNos);
        
        calculateBunchingRMSE(stopBunchingRMSE, simulator.busGenerator->peakBusVec, 3600.0 / config.meanHeadway);
        addVector(stopBunchingEach, stopBunchingRMSE);
        
        simulator.reset();
    }

    int totalRuns = 0;
    if (realRuns <= 0){
        totalRuns = testRuns;
    }else{
        totalRuns = testRuns + realRuns;
    }
    multiplyVector(stopDelayEach, 1.0/60.0/totalRuns);
    multiplyVector(stopServiceEach, 1.0/60.0/totalRuns);
    multiplyVector(stopBunchingEach, 1.0/60.0/totalRuns);
    multiplyVector(stopEntryDelayEach, 1.0/60.0/totalRuns);
    multiplyVector(stopExitDelayEach, 1.0/60.0/totalRuns);
    multiplyVector(stopPaxNoEach, 1.0/totalRuns);
    
    
    // shout out to the python console and log file

    double delayCumSum = 0.0;
    double entryDelayCumSum = 0.0;
    double exitDelayCumSum = 0.0;
    double servTimeCumSum = 0.0;
    double bunchingRMSE = 0.0;
    double paxNo = 0.0;
    for (int s = 0; s < stopSize+1; s++) {
        for(int i = 1; i < argc; ++i) std::cout << argv[i] << " ";
        std::cout << s << " ";

        // last element is the delay in the consolidation stop
        if (s == 0){
            delayCumSum += stopDelayEach[stopSize];
            bunchingRMSE = 0.0;
            paxNo = stopPaxNoEach[0];
        }else{
            delayCumSum += stopDelayEach[s-1];
            entryDelayCumSum += stopEntryDelayEach[s-1];
            exitDelayCumSum += stopExitDelayEach[s-1];
            bunchingRMSE = stopBunchingEach[s-1];
            paxNo = stopPaxNoEach[s-1];
        }

        if (s != 0) {
            servTimeCumSum += stopServiceEach[s-1];
        }
        std::cout << entryDelayCumSum << " " << exitDelayCumSum << " " << delayCumSum << " " << servTimeCumSum << " " << bunchingRMSE << " " <<  paxNo << std::endl;
    }
}








