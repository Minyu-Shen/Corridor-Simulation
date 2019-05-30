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
#include "Stats.cpp"

class Stats;

// declare:
void normal(int argc, char * argv[], int objective);
//void correlation_analysis(int argc, char * argv[]);

// one simulation round
void oneRun(Corridor simulator, SimulationConfig config, double warmDuration, double peakDuration, double warmTotalPaxRate, std::shared_ptr<Stats> stats, std::map<int, std::vector<double>> &estimatingRunsMap, int r);

//

int main(int argc, char * argv[]) {
    // objective: 0->normal, 1->correlation analysis
    normal(argc, argv, 0);
}

void oneRun(Corridor simulator, SimulationConfig config, double warmDuration, double peakDuration, double warmTotalPaxRate, std::shared_ptr<Stats> stats, std::map<int, std::vector<double>> &estimatingRunsMap, int r){
    int stopSize = simulator.busGenerator->kStop;
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
    std::vector<double> stopDelays (stopSize+1);std::vector<double> stopServices (stopSize);
    std::vector<double> stopBunchingRMSE (stopSize);std::vector<double> stopEntryDelays (stopSize);
    std::vector<double> stopExitDelays (stopSize);std::vector<double> stopPaxNos (stopSize);
    
    // compute the mean of all the buses in one simulation round
    computeMeanDelay(stopDelays, stopServices, stopEntryDelays, stopExitDelays, stopPaxNos, simulator.busGenerator->peakBusVec);
    /* bunching RMSE estimations */
    calculateBunchingRMSE(stopBunchingRMSE, simulator.busGenerator->peakBusVec, 3600.0 / config.meanHeadway);
    addVector(stats->totalDelay, stopDelays); addVector(stats->meanService, stopServices);
    addVector(stats->entryDelay, stopEntryDelays); addVector(stats->exitDelay, stopExitDelays);
    addVector(stats->paxNo, stopPaxNos); addVector(stats->bunchingRMSE, stopBunchingRMSE);
    estimatingRunsMap.insert(std::make_pair(r, stopDelays));
    
    simulator.reset();
    //        writeJsonToFile(simulator.jsObject);
    //        std::cout << r << std::endl;
}

void normal(int argc, char *argv[], int objective){
    SimulationConfig config(argc, argv);
    Corridor simulator = SimulationConfig::CorridorBuilder(config);
    
    int stopSize = simulator.busGenerator->kStop;
    double warmDuration = 3600.0 * 1.5;
    double peakDuration = 3600.0 * 5.5;
    double warmTotalPaxRate = 100.0; // pax/hr
    auto stats = std::make_shared<Stats>(0, stopSize);
    
    /* ---------- test runs ---------- */
    double testRuns = 200; //200
    
    // key is sample no, vector contains the sampels
    // to determine the needed simulation rounds
    std::map<int, std::vector<double>> estimatingRunsMap;
    
    for (int r = 0; r < testRuns; r++) {
        oneRun(simulator, config, warmDuration, peakDuration, warmTotalPaxRate, stats, estimatingRunsMap, r);
    }
    // estimate num of simulations needed
    int realRuns = computeRuns(estimatingRunsMap) - testRuns;
    //    std::cout << "real runs: " << realRuns << std::endl;
    
    /* ---------- real runs ---------- */
    for (int r = 0; r < realRuns; r++) {
        oneRun(simulator, config, warmDuration, peakDuration, warmTotalPaxRate, stats, estimatingRunsMap, r);
    }
    
    int totalRuns = 0;
    if (realRuns <= 0){
        totalRuns = testRuns;
    }else{
        totalRuns = testRuns + realRuns;
    }
    multiplyVector(stats->totalDelay, 1.0/60.0/totalRuns);
    multiplyVector(stats->meanService, 1.0/60.0/totalRuns);
    multiplyVector(stats->bunchingRMSE, 1.0/60.0/totalRuns);
    multiplyVector(stats->entryDelay, 1.0/60.0/totalRuns);
    multiplyVector(stats->exitDelay, 1.0/60.0/totalRuns);
    multiplyVector(stats->paxNo, 1.0/totalRuns);


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
            delayCumSum += stats->totalDelay[stopSize];
            bunchingRMSE = 0.0;
            paxNo = stats->paxNo[0];
        }else{
            delayCumSum += stats->totalDelay[s-1];
            entryDelayCumSum += stats->entryDelay[s-1];
            exitDelayCumSum += stats->exitDelay[s-1];
            bunchingRMSE = stats->bunchingRMSE[s-1];
            paxNo = stats->paxNo[s-1];
        }

        if (s != 0) {
            servTimeCumSum += stats->meanService[s-1];
        }
        std::cout << entryDelayCumSum << " " << exitDelayCumSum << " " << delayCumSum << " " << servTimeCumSum << " " << bunchingRMSE << " " <<  paxNo << std::endl;
    }
}





