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
    double warmDuration = 3600.0 * 2;
    double peakDuration = 3600.0 * 5;
    //    double warmDuration = 3600.0 * 0.25;
    //    double peakDuration = 3600.0 * 0.25;
    double warmTotalPaxRate = 100.0; // pax/hr
    
//    clock_t begin = clock();
//    clock_t end = clock();
//    double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
//    std::cout << elapsed_secs << std::endl;
    
    /* ---------- test runs ---------- */
    double testRuns = 200;
    std::vector<double> stopDelayEach (stopSize+1);
    std::vector<double> stopServiceEach (stopSize);
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
        computeMeanDelay(stopDelays, stopServices, simulator.busGenerator->peakBusVec);
        estimatingRunsMap.insert(std::make_pair(r, stopDelays));
        addVector(stopDelayEach, stopDelays);
        addVector(stopServiceEach, stopServices);
        simulator.reset();
//        writeJsonToFile(simulator.jsObject);
//        std::cout << r << std::endl;
        
    } // loop ends
    
    // estimate num of simulations needed
    int realRuns = computeRuns(estimatingRunsMap) - testRuns;
//    std::cout << "real runs: " << realRuns << std::endl;
    
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
        //        std::vector<double> stopDelays = simulator.collectDelayAsArray();
        computeMeanDelay(stopDelays, stopServices, simulator.busGenerator->peakBusVec);
        addVector(stopDelayEach, stopDelays);
        addVector(stopServiceEach, stopServices);
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

    // shout out to the python console and log file

    double delayCumSum = 0.0;
    double servTimeCumSum = 0.0;
    for (int s = 0; s < stopSize+1; s++) {
        for(int i = 1; i < argc-1; ++i) std::cout << argv[i] << " ";
        std::cout << s << " ";

        // last element is the delay in the consolidation stop
        if (s == 0) delayCumSum += stopDelayEach[stopSize];
        else delayCumSum += stopDelayEach[s-1];
        if (s != 0) {
            servTimeCumSum += stopServiceEach[s-1];
        }
        std::cout << delayCumSum << " " << servTimeCumSum << std::endl;
    }
}








