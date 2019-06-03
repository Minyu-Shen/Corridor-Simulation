//
//  SimulationConfig.cpp
//  Corridor-Simulation
//
//  Created by samuel on 8/2/18.
//  Copyright © 2018 samuel. All rights reserved.
//

#include "SimulationConfig.hpp"


SimulationConfig::SimulationConfig(int argc, char *argv[]){
    
    // number of berthSize
    int argv_1 = std::stoi(argv[1]);

    // convoy size
    int argv_2 = std::stoi(argv[2]);

    // number of line
    int argv_3 = std::stoi(argv[3]);

    // average link travel time
    double argv_4 = std::stod(argv[4]);

    // total bus flow (bus arrival mean), buses/hr
    double argv_5 = std::stod(argv[5]);

    // total pax flow
    double argv_6 = std::stod(argv[6]);

    // bus arrival c.v.
    double argv_7 = std::stod(argv[7]);

    // link travel time c.v.
    double argv_8 = std::stod(argv[8]);

    // dispatch strategy
    int argv_9 = std::stoi(argv[9]);

    // number of stop
    int argv_10 = std::stoi(argv[10]);

    // capacity
    double argv_11 = std::stod(argv[11]);

    // alighting prob
    double argv_12 = std::stod(argv[12]);

    // common passenger ratio within the group
    double argv_13 = std::stod(argv[13]);

    // common passenger ratio for all the lines
    double argv_14 = std::stod(argv[14]);

    // which queueing rule, 0-> parallel, 1->FIFO
    int argv_15 = std::stoi(argv[15]);
    
//    int argv_1 = 3, argv_2 = 3, argv_3 = 3, argv_9 = 0, argv_10 = 18, argv_15 = 0;
//    double argv_4 = 60.0, argv_5 = 100.0, argv_6 = 600, argv_7 = 0.4, argv_8 = 0.0, argv_13 = 0.0, argv_12 = 0.2, argv_11 = 120, argv_14 = 0.0;
    
    berthSize = argv_1; kStop = argv_10; convoySize = argv_2;
    kLine = argv_3; totalPaxArrRate = argv_6/3600.0; // seconds/bus
    meanHeadway = 1 / (argv_5 / 3600.0 / kLine); // seconds/bus
    cvHeadway = argv_7; travelMean = argv_4; travelStd = argv_8*argv_4;
    capacity = argv_11; alightingProb = argv_12;
    cp_ratio = argv_13; cp_ratio_all = argv_14;
    
    //enum class DispatchMode{Normal, Convoy, Serial, SerialFixHeadway, ConvoyFixHeadway, SingleNormal};
    switch (argv_9) {
        case 0: dispatchMode = DispatchMode::Normal;
            break;
        case 1: dispatchMode = DispatchMode::Convoy;
            break;
        case 2: dispatchMode = DispatchMode::Serial;
            break;
        case 3: dispatchMode = DispatchMode::SerialFixHeadway;
            break;
        case 4: dispatchMode = DispatchMode::ConvoyFixHeadway;
            break;
        default: break;
    }
    // enum class QueuingRules{Parallel, FIFO, LimitedOvertaking, FreeOvertaking, FreeOvertakingWithBlock};
    switch (argv_15) {
        case 0:
            queuingRule = QueuingRules::Parallel;
            break;
        case 1:
            queuingRule = QueuingRules::FIFO;
        default:
            break;
    }
    enterStopType = EnteringTypes::Normal;
    busArriveMode = BusArriveCorridorMode::Gaussian;
    
    // hyper paras
    boardingRate = 1/4.0; alightingRate = 1/2.0;
//    alightingProb = 0.1;
//    capacity = 1000000.0;
    initialPax = 0.0; // 30.0
}

Corridor SimulationConfig::CorridorBuilder(SimulationConfig config){
    Corridor simulator = Corridor();
    // pax arrival rate for each line
    std::map<int, double> lineDemandMap;
    // mean arrival headway for each line
    std::map<int, double> busLineMeanHeadway;
    // only for gaussian bus arrival
    std::map<int, double> busLineCVHeadway;
    for (int k = 0; k < config.kLine; k++) {
        lineDemandMap.insert(std::make_pair(k, config.totalPaxArrRate / config.kLine));
        busLineMeanHeadway.insert(std::make_pair(k, config.meanHeadway));
        busLineCVHeadway.insert(std::make_pair(k, config.cvHeadway));
    }
    
    // create line-group assignment
    // line No. -> group No.
    std::map<int, int>lineGroupAMap;
    int m = (int)config.kLine/config.berthSize;
    int temp_loop = 0, current_group = 0;
    for (auto &p: lineDemandMap){
        // generating the line-group assignment plan
        lineGroupAMap.insert(std::make_pair(p.first, current_group));
        temp_loop ++;
        if (temp_loop >= m) {
            current_group++;
            temp_loop = 0;
        }
    }
    
    
    // creating bus generator
    auto bg = std::make_shared<BusGenerator>(busLineMeanHeadway, busLineCVHeadway, config.busArriveMode, config.dispatchMode, config.boardingRate, config.alightingRate, config.capacity, config.alightingProb, config.initialPax, config.kStop, lineGroupAMap, config.convoySize);
    //  Check Dispatch Mode
    if (config.dispatchMode == DispatchMode::Convoy || config.dispatchMode == DispatchMode::ConvoyFixHeadway) { // convoy case
        // creating bus stops
        std::vector<std::shared_ptr<PaxConvoyStop>>stops(config.kStop);
        for (int i = 0; i < config.kStop; i++) {
            stops[i] = std::make_shared<PaxConvoyStop>(i, config.berthSize, lineDemandMap, config.cp_ratio, config.cp_ratio_all, lineGroupAMap);
        }
        // creating links
        // link number is equal to stop number
        std::vector<std::shared_ptr<Link>> links(config.kStop);
        for (int i = 0; i < config.kStop; i++) {
            links[i] = std::make_shared<Link> (i, config.travelMean, config.travelStd, true);
        }
        // connect bus generator with the first link, and links with stops
        bg->nextLink = links[0];
        for (int i = 0; i < config.kStop; i++) {
            links[i]->nextConovyStop = stops[i];
        }
        for (int i = 0; i < config.kStop-1; i++) { // when i==kStop-1, nextLink is nullptr
            stops[i]->nextLink = links[i+1];
        }
        // assembling all pieces into a corridor
        simulator.addBusGenerator(bg);
        for (int i = 0; i < config.kStop; i++) {
            simulator.addLink(links[i]);
            simulator.addConvoyStop(stops[i]);
        }
        return simulator;
    }else {
        
        // creating bus stops
        std::vector<std::shared_ptr<PaxStop>>stops(config.kStop);
        for (int i = 0; i < config.kStop; i++) {
            stops[i] = std::make_shared<PaxStop>(i, config.berthSize, lineDemandMap, config.enterStopType, config.queuingRule, config.cp_ratio, config.cp_ratio_all, lineGroupAMap);
        }
        // creating links
        // link number is equal to stop number
        std::vector<std::shared_ptr<Link>> links(config.kStop);
        for (int i = 0; i < config.kStop; i++) {
            links[i] = std::make_shared<Link> (i, config.travelMean, config.travelStd, false);
        }
        // connect bus generator with the first link, and links with stops
        bg->nextLink = links[0];
        for (int i = 0; i < config.kStop; i++) {
            links[i]->nextStop = stops[i];
        }
        for (int i = 0; i < config.kStop-1; i++) { // when i==kStop-1, nextLink is nullptr
            stops[i]->nextLink = links[i+1];
        }
        // assembling all pieces into a corridor
        simulator.addBusGenerator(bg);
        for (int i = 0; i < config.kStop; i++) {
            simulator.addLink(links[i]);
            simulator.addStop(stops[i]);
        }
        return simulator;
    }
}










