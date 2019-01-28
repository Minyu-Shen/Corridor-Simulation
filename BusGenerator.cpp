//
//  BusGenerator.cpp
//  Corridor-Simulation
//
//  Created by samuel on 7/11/18.
//  Copyright © 2018 samuel. All rights reserved.
//

#include "BusGenerator.hpp"
#include "arena.hpp"
#include <iostream>
#include "Link.hpp"
#include "Convoy.hpp"
#include "PaxStop.hpp"

BusGenerator::BusGenerator(std::map<int,double> lineMH, std::map<int,double> lineCH, BusArriveCorridorMode arrMode, DispatchMode disMode, double bd_rt, double al_rt, double cpt, double al_prob, double init_pax, int stop_no, int cSize){
    
    dispatchMode = disMode; arriveMode = arrMode;
    for (auto &p: lineMH){
        lineMeanHeadway.insert(std::make_pair(p.first, p.second));
        lines.push_back(p.first);
        // initializing the busQueues
        std::deque<std::shared_ptr<Bus>> oneDeque;
        busQueues.insert(std::make_pair(p.first, oneDeque));
        lastDeparture.insert(std::make_pair(p.first, 0.0));
    }
    // if the arrival is gaussian, need the c.v.
    if (arrMode == BusArriveCorridorMode::Gaussian) {
        for (auto &p: lineCH){
            lineCvHeadway.insert(std::make_pair(p.first, p.second));
        }
    }
    nextLink = nullptr;
    boardingRate = bd_rt; alightingRate = al_rt;
    capacity = cpt; alightingProb = al_prob;
    totalBus = 0; totalConvoy = 0;
    initialPax = init_pax; convoySize = cSize;
    lastDispatchLine = -1; lastDepartureConvoy = 0.0;
    kStop = stop_no; warmupTime= 0.0;
    totlePeakBus = 0;
}

void BusGenerator::reset(){
    for (auto ln:lines){
        busQueues[ln].clear();
        busSchedule.clear();
        lastDeparture[ln] = 0.0;
    }
    peakBusVec.clear();
    totalBus = 0;
    lastDispatchLine = -1;
    lastDepartureConvoy = 0.0;
    totalConvoy = 0;
    totlePeakBus = 0;
    
}

void BusGenerator::schedule(double warm_t, double peak_t){
    double simTotalTime = warm_t + peak_t;
    warmupTime = warm_t;
    for(auto ln: lines){
        if (arriveMode == BusArriveCorridorMode::Poisson) {
            double hdw = lineMeanHeadway[ln];
            std::deque<double> x;
            expTime(x, hdw, simTotalTime);
            busSchedule.insert(std::make_pair(ln, x));
        } else if (arriveMode == BusArriveCorridorMode::Gaussian){
            double hdw = lineMeanHeadway[ln];
            double cv = lineCvHeadway[ln];
            std::deque<double> x;
//            gaussianTime(x, hdw, cv, simTotalTime);
            gaussianTimeIndepent(x, hdw, cv, simTotalTime);
            busSchedule.insert(std::make_pair(ln, x));
        }else{
            // constant bus arrival, future ...
        }
    }
}

void BusGenerator::arrival(double time){
    for (auto ln: lines){
        while (!busSchedule[ln].empty() && busSchedule[ln].front() <= time) {
            auto bus = std::make_shared<Bus>(totalBus, ln, capacity, boardingRate, alightingRate, alightingProb, initialPax, kStop);
            busSchedule[ln].pop_front();
            if (busSchedule[ln].front() > warmupTime){
                peakBusVec.push_back(bus);
                bus->isPeak = true;
                totlePeakBus ++;
            }
            busQueues[ln].push_back(bus);
            totalBus += 1;
        }
    }
}

void BusGenerator::dispatch(double time){
//    if (time <= warmupTime) {
//        normalDispatch(time);
//    }else{
        if (dispatchMode == DispatchMode::Normal) {
            normalDispatch(time);
        } else if (dispatchMode == DispatchMode::Convoy){
            convoyDispatch(time);
        } else if (dispatchMode == DispatchMode::ConvoyFixHeadway){
            convoyFixHeadwayDispatch(time);
        } else if (dispatchMode == DispatchMode::Serial){
            serialDispatch(time);
        } else if (dispatchMode == DispatchMode::SerialFixHeadway){
            serialFixHeadwayDispatch(time);
        } else{
            
        }
//    }
}

// dispatch the first bus into the first link
void BusGenerator::normalDispatch(double time){
    // shuffle the line array
    // to reduce the effort of sorting the max among all the lines
    // needed???
    std::random_shuffle(lines.begin(), lines.end());
    for(auto ln: lines){
        // front is the first arrived
        while (!busQueues[ln].empty()) {
            dispatchOneToLink(ln);
        }
    }
}

void BusGenerator::dispatchOneToLink(int ln){
    auto bus = busQueues[ln].front();
    nextLink->busEnteringLink(bus);
    busQueues[ln].pop_front();
}

void BusGenerator::serialDispatch(double time){
    int n = (int)lines.size();
    int ln = lines[(lastDispatchLine+1) % n];
    while (!busQueues[ln].empty()) {
        dispatchOneToLink(ln);
        lastDispatchLine = (lastDispatchLine+1) % n;
        // check if can dispatch next line immediately
        ln = (lastDispatchLine+1) % n;
    }
}

void BusGenerator::serialFixHeadwayDispatch(double time){
    int n = (int)lines.size();
    int ln = (lastDispatchLine+1) % n;
    while (!busQueues[ln].empty()) {
        if (time-lastDeparture[ln] >= lineMeanHeadway[ln]) {
            dispatchOneToLink(ln);
            lastDispatchLine = (lastDispatchLine+1) % n;
            lastDeparture[ln] = time;
            // check if can dispatch next line immediately
            ln = (lastDispatchLine+1) % n;
        }else{
            break;
        }
    }
}


// dispatch the convoy into the first link
void BusGenerator::convoyDispatch(double time){
    std::vector<int> convoyLineVector;
    std::random_shuffle(lines.begin(), lines.end());
    // shuffle is not good, change it in the future ...
    
    for (auto &ln: lines){
        if (!busQueues[ln].empty()) {
            convoyLineVector.push_back(ln);
        }
        if (convoyLineVector.size() == convoySize) break;
    }
    
    if (convoyLineVector.size() == convoySize) { // ready to be convoyed
        std::vector<std::shared_ptr<Bus>> convoyVector;
        for (auto ln: convoyLineVector){
            convoyVector.push_back(busQueues[ln].front());
            busQueues[ln].pop_front();
//            totalBus += 1;
        }
        auto convoy = std::make_shared<Convoy>(totalConvoy, convoySize, convoyVector, convoyLineVector);
        nextLink->convoyEnteringLink(convoy);
        totalConvoy += 1;
    }
    convoyLineVector.clear();
}

void BusGenerator::convoyFixHeadwayDispatch(double time){
    // future ... change
    std::random_shuffle(lines.begin(), lines.end());
    if (time-lastDepartureConvoy >= lineMeanHeadway[0]) {
        std::vector<int> convoyLineVector;
        for (auto &ln: lines){
            if (!busQueues[ln].empty()) {
                convoyLineVector.push_back(ln);
            }
            if (convoyLineVector.size() == convoySize) break;
        }
        if (convoyLineVector.size() == convoySize) { // ready to be convoyed
            std::vector<std::shared_ptr<Bus>> convoyVector;
            for (auto ln: convoyLineVector){
                convoyVector.push_back(busQueues[ln].front());
                busQueues[ln].pop_front();
//                totalBus += 1;
            }
            auto convoy = std::make_shared<Convoy>(totalConvoy, convoySize, convoyVector, convoyLineVector);
            nextLink->convoyEnteringLink(convoy);
            totalConvoy += 1;
            lastDepartureConvoy = time;
        }
        convoyLineVector.clear();
    }
}

void BusGenerator::updateBusStats(){
//    for (auto ln:lines){
//        int blockCount = 0;
//        for (int i = int(busQueues[ln].size())-1; i >= 0; i--) {
//            auto bus = busQueues[ln][i];
//            if (bus->isPeak) {
//                 blockCount ++;
//            }else{
//                break;
//            }
//        }
//        consolidDelays += 1.0 * blockCount;
//    }
    
    for (auto ln: lines){
        for (auto &bus: busQueues[ln]){
            if (bus->isPeak) {
                bus->delayAtEachStop[-1] += 1.0;
            }
        }
    }
}


void BusGenerator::writeBusQueueToJson(nlohmann::json &j, double time){
    std::string timeString = std::to_string(time);
    for (auto ln:lines){
        std::string lineIDStr = std::to_string(ln);
        if (!busQueues[ln].empty()) {
            j["bus_queues"][lineIDStr][timeString] = int(busQueues[ln].size());
        }else{
            j["bus_queues"][lineIDStr][timeString] = 0;
        }
    }
}















