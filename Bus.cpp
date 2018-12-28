//
//  Bus.cpp
//  Corridor-Simulation
//
//  Created by samuel on 7/7/18.
//  Copyright © 2018 Samuel. All rights reserved.
//

#include "Bus.hpp"
#include "Queues.hpp"
#include "arena.hpp"
#include <iostream>

Bus::Bus(int id, int bl, double cpt, double bd_rt, double al_rt, double al_prob, double init_pax, int kStop){
    busLine = bl; busID = id;
    std::map<int, double> dms; capacity = cpt;
    boardingPaxPerDelata = bd_rt; alightingPaxPerDelta = al_rt;
    alightProbability = al_prob;
    dms.insert(std::make_pair(bl, 0.0));
    // initializing the paxQueues on the bus
    // the queues on the bus don't need the arrival rate (value)
    // just one key for now
    paxOnBusQueue = std::make_shared<Queues>(dms);
    // initializing the pax queue
    paxOnBusQueue->increase(bl, init_pax);
    kPax = init_pax;
    for (int k = 0; k < kStop; k++){
        delayAtEachStop.insert(std::make_pair(k, 0.0));
        delayOnEachLink.insert(std::make_pair(k, 0.0));
        serviceTimeAtEachStop.insert(std::make_pair(k, 0.0));
    }
    // -1 means the consolidation stop
    delayAtEachStop.insert(std::make_pair(-1, 0.0));
    totalTimeInService = 0.0;
    totalDelays = 0.0;
}

double Bus::boarding(int ln, double paxOnStop){
    double boardPax = std::min(paxOnStop, boardingPaxPerDelata);
    boardPax = std::min(boardPax, remainSpace());
    kPax += boardPax;
    paxOnBusQueue->increase(ln, boardPax);
    return boardPax;
}

// determine the alighting pax when arriving at each stop
void Bus::determineAlightingPaxNo(int ln){
    double paxOnBus = paxOnBusQueue->query(ln);
    alightingPaxEachStop = double(binomial(int(paxOnBus), alightProbability));
}

void Bus::alighting(int ln){
    if (alightingPaxEachStop > 0) {
        double actualAlightPax = std::min(alightingPaxEachStop, alightingPaxPerDelta);
        paxOnBusQueue->decrease(ln, actualAlightPax);
        alightingPaxEachStop -= actualAlightPax;
        kPax -= actualAlightPax;
    }
}

double Bus::remainSpace(){
//    std::cout << capacity-kPax << std::endl;
    return capacity-kPax;
}





