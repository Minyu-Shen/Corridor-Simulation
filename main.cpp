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
void oneRun(Corridor simulator, SimulationConfig config, double warmDuration, double peakDuration, double warmTotalPaxRate, std::shared_ptr<Stats> stats, std::map<int, std::vector<double>> &estimatingRunsMap, int r, bool isTest, int objective);

int main(int argc, char * argv[]) {
    // objective: 0->normal, 1->correlation analysis
    normal(argc, argv, 0);
}

void normal(int argc, char *argv[], int objective){
    SimulationConfig config(argc, argv);
    Corridor simulator = SimulationConfig::CorridorBuilder(config);
    
    int stopSize = simulator.busGenerator->kStop;
    double warmDuration = 3600.0 * 1.0;
    double peakDuration = 3600.0 * 6.0;
    double warmTotalPaxRate = 100.0; // pax/hr
    auto stats = std::make_shared<Stats>(objective, stopSize);
    
    /* ---------- test runs ---------- */
    int testRuns = std::stoi(argv[16]);; //200
    
    // key is sample no, vector contains the sampels
    // to determine the needed simulation rounds
    std::map<int, std::vector<double>> estimatingRunsMap;
    
    for (int r = 0; r < testRuns; r++) {
        oneRun(simulator, config, warmDuration, peakDuration, warmTotalPaxRate, stats, estimatingRunsMap, r, true, objective);
    }
    // estimate num of simulations needed
    int realRuns = computeRuns(estimatingRunsMap) - testRuns;
//    std::cout << realRuns << std::endl;

    /* ---------- real runs ---------- */
    for (int r = 0; r < realRuns; r++) {
        oneRun(simulator, config, warmDuration, peakDuration, warmTotalPaxRate, stats, estimatingRunsMap, r, false, objective);
    }

    int totalRuns = testRuns +  max(0, realRuns);

    stats->convertUnit(totalRuns, objective);
    stats->printToPython(argc, argv, objective);
    
}

void oneRun(Corridor simulator, SimulationConfig config, double warmDuration, double peakDuration, double warmTotalPaxRate, std::shared_ptr<Stats> stats, std::map<int, std::vector<double>> &estimatingRunsMap, int r, bool isTest, int objective){
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
    std::vector<double> stopDelays (stopSize+1); std::vector<double> stopDelayStds (stopSize+1);
    std::vector<double> stopPaxNos (stopSize);
    std::vector<double> meanDwellTimes (stopSize); std::vector<double> cvDwellTimes (stopSize);
    std::vector<double> stopExitDelays (stopSize); std::vector<double> stopEntryDelays (stopSize);
    std::vector<double> arrivalHeadwayMean (stopSize); std::vector<double> arrivalHeadwayCv (stopSize);
    std::vector<double> entryHeadwayMean (stopSize); std::vector<double> entryHeadwayCv (stopSize);
    std::vector<double> departureHeadwayMean (stopSize); std::vector<double> departureHeadwayCv (stopSize);
    
    // compute the mean of all the buses in one simulation round
    computeMeanDelay(stopDelays, stopDelayStds, meanDwellTimes, cvDwellTimes, stopEntryDelays, stopExitDelays, stopPaxNos, simulator.busGenerator->peakBusVec);
    /* bunching RMSE estimations */
//    calculateBunchingRMSE(stopBunchingRMSE, stopDepartureRMSE, simulator.busGenerator->peakBusVec, 3600.0 / config.meanHeadway);
    calculateHeadwayVariation(config.kLine, arrivalHeadwayMean, arrivalHeadwayCv, entryHeadwayMean, entryHeadwayCv, departureHeadwayMean, departureHeadwayCv, simulator.busGenerator->peakBusVec);
    
    if (objective == 0) stats->updateNormal(stopDelays, meanDwellTimes, stopEntryDelays, stopExitDelays, stopPaxNos, entryHeadwayCv);
    else stats->updateCorr(meanDwellTimes, cvDwellTimes, arrivalHeadwayMean, arrivalHeadwayCv, entryHeadwayMean, entryHeadwayCv, departureHeadwayMean, departureHeadwayCv, stopDelays, stopDelayStds);
    
    if (isTest) {
        estimatingRunsMap.insert(std::make_pair(r, stopDelays));
    }
    

    simulator.reset();
//    writeJsonToFile(simulator.jsObject);
}

