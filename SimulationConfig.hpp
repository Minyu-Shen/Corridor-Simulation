//
//  SimulationConfig.hpp
//  Corridor-Simulation
//
//  Created by samuel on 8/2/18.
//  Copyright © 2018 samuel. All rights reserved.
//

#ifndef SimulationConfig_hpp
#define SimulationConfig_hpp

#include <iostream>
#include "Corridor.hpp"
#include "arena.hpp"
#include "BusGenerator.hpp"
#include "PaxStop.hpp"
#include "PaxConvoyStop.hpp"
#include "Link.hpp"

enum class BusArriveCorridorMode;
enum class DispatchMode;
enum class QueuingRules;
enum class EnteringTypes;

class SimulationConfig{

public:
    
    // berth size, stop No., convoy size
    int berthSize, kStop, convoySize;
    
    // board and alight
    double boardingRate, alightingRate, alightingProb;
    
    // capacity and initialPax entering the corridor
    double capacity, initialPax;
    
    // common passenger ratio
    double cp_ratio;
    
    // line configuration
    int kLine; double totalPaxArrRate;
    
    // link configuration
    double linkLength, travelMean, travelStd;
    
    // bus arrival paras
    double meanHeadway, cvHeadway;
    
    // bus arrvial mode to the consolidation
    BusArriveCorridorMode busArriveMode;
    
    // dispatch mode at the beginning
    DispatchMode dispatchMode;
    
    // queuing rules at each stop
    QueuingRules queuingRule;
    
    // bus entering type, normal nor allocated
    EnteringTypes enterStopType;
    
    
    // for static stop:
    // berth allocation plan
    std::map<int, int> berthAllocationPlan;
    
    // line arrival mean
    std::map<int, double>busLineMeanHeadway;
    
    // line arrival cv
    std::map<int, double>busLineCVHeadway;
    
    // line service time mean
    std::map<int, double>busLineMeanServTime;
    
    // line service time cv
    std::map<int, double>busLineCvServTime;
    
    // constructor
    SimulationConfig(int argc, char *argv[]);
  
    // build an Corridor from the given configuration
    static Corridor CorridorBuilder(SimulationConfig config);
};

#endif /* SimulationConfig_hpp */
