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

BusGenerator::BusGenerator(std::map<int,double> lineMH, std::map<int,double> lineCH, BusArriveCorridorMode arrMode, DispatchMode disMode, double bd_rt, double al_rt, double cpt, double al_prob, double init_pax, int stop_no, const std::map<int, int>lineGroupCMap, const std::map<int, int>lineGroupSMap, int serialGroupSize, int convoyGroupSize, int strtgy){
    
    // 0->headway measured within group, 2->headway measured in all lines
    dispatchMode = disMode; arriveMode = arrMode; strategy = strtgy;
    int no_line = (int)lineMH.size();
    // if line number is a multiple integer of berth number
    // else, future ...
    for (auto &p: lineMH){
        lineMeanHeadway.insert(std::make_pair(p.first, p.second));
        lines.push_back(p.first);
    }

    // initializing the groupQueues, # of group = convoy size
    for (int group = 0; group < convoyGroupSize; group++) {
        std::deque<std::shared_ptr<Bus>> oneGroupDeque;
        groupQueues.insert(std::make_pair(group, oneGroupDeque));
        groups.push_back(group);
    }
    lineGroupConvoyMap = lineGroupCMap;
    
    // initializing the serialGroupQueues, # of group = inputs
    for (int group = 0; group < serialGroupSize; group++) {
        std::deque<std::shared_ptr<Bus>> oneGroupDeque;
        serialGroupQueues.insert(std::make_pair(group, oneGroupDeque));
        serialGroups.push_back(group);
        lastDepartureTimeSerialGroupMap.insert(std::make_pair(group, 0.0));
        lastDepartureTimeSerialGroupMap.insert(std::make_pair(-1, 0.0)); // for the first dispatch bus
    }
    lineGroupSerialMap = lineGroupSMap;
    
    m = (int)no_line / convoyGroupSize;
    int n = (int)no_line / serialGroupSize;
    if (strategy == 0) {
        lineFixedHeadway = lineMeanHeadway[0] / n; // for now, all the lines are same
        convoyFixedHeadway = lineMeanHeadway[0] / m; // for now, also equal to: H/m
    }else{ // strategy == 2
        // ...
        lineFixedHeadway = (lineMeanHeadway[0] / no_line);
        convoyFixedHeadway = lineMeanHeadway[0] / m;
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
    initialPax = init_pax; convoySize = convoyGroupSize;
    lastDispatchGroup = -1; lastDepartureTimeConvoy = 0.0;
    kStop = stop_no; warmupTime= 0.0;
    totlePeakBus = 0;
}

void BusGenerator::reset(){
    if (dispatchMode == DispatchMode::Normal || dispatchMode == DispatchMode::SerialFixHeadway) {
        for (auto grp:serialGroups){
            serialGroupQueues[grp].clear();
            lastDepartureTimeSerialGroupMap[grp] = 0.0;
            lastDepartureTimeSerialGroupMap[-1] = 0.0;
            lastDispatchGroup = -1;
        }
    }else{ // convoy
        for (auto grp:groups){
            groupQueues[grp].clear();
        }
        lastDepartureTimeConvoy = 0.0;
    }
    
    busSchedule.clear();
    peakBusVec.clear();
    totalBus = 0;
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
            if (dispatchMode == DispatchMode::Normal || dispatchMode == DispatchMode::SerialFixHeadway) {
                serialGroupQueues[lineGroupSerialMap[ln]].push_back(bus);
            }else{
                groupQueues[lineGroupConvoyMap[ln]].push_back(bus);
            }
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
        } else if (dispatchMode == DispatchMode::SerialFixHeadway){
            serialFixHeadwayDispatch(time);
        } else{
            
        }
//    }
}

// dispatch the first bus into the first link
void BusGenerator::normalDispatch(double time){
    // shuffle the group array
    // to reduce the effort of sorting the max among all the groups
    // use serial group for normal dispatch
    std::random_shuffle(serialGroups.begin(), serialGroups.end());
    for(auto grp: serialGroups){
        while (!serialGroupQueues[grp].empty()){
            dispatchOneToLink(grp);
        }
    }
}

void BusGenerator::dispatchOneToLink(int which){
    // which is line or group, denpent on strategy
    auto bus = serialGroupQueues[which].front();
    nextLink->busEnteringLink(bus);
    serialGroupQueues[which].pop_front();
}

void BusGenerator::serialFixHeadwayDispatch(double time){
    if (strategy == 0) {
        int grp_total = (int)serialGroups.size();
        int next_grp = serialGroups[(lastDispatchGroup+1) % grp_total];
        while (!serialGroupQueues[next_grp].empty()) {
            if (time-lastDepartureTimeSerialGroupMap[next_grp] >= lineFixedHeadway) {
                dispatchOneToLink(next_grp);
                lastDispatchGroup = (lastDispatchGroup+1) % grp_total;
                lastDepartureTimeSerialGroupMap[next_grp] = time;
                // check if can dispatch next group immediately
                next_grp = (lastDispatchGroup+1) % grp_total;
            }else{
                break;
            }
        }
    }else{ // strategy == 2
        // ...
        int grp_total = (int)serialGroups.size();
        int next_grp = serialGroups[(lastDispatchGroup+1) % grp_total];
        while (!serialGroupQueues[next_grp].empty()) {
            if (time-lastDepartureTimeSerialGroupMap[lastDispatchGroup] >= lineFixedHeadway) {
                dispatchOneToLink(next_grp);
                lastDispatchGroup = (lastDispatchGroup+1) % grp_total;
                lastDepartureTimeSerialGroupMap[next_grp] = time;
                // check if can dispatch next group immediately
                next_grp = (lastDispatchGroup+1) % grp_total;
            }else{
                break;
            }
        }
    }
}

// dispatch the convoy into the first link
void BusGenerator::convoyDispatch(double time){
    std::vector<int> convoyLineVector;
    for (auto &grp: groups){
        if (!groupQueues[grp].empty()){
            convoyLineVector.push_back(groupQueues[grp].front()->busLine);
        }
        if (convoyLineVector.size() == convoySize) break;
    }
    if (convoyLineVector.size() == convoySize) { // ready to be convoyed
        std::vector<std::shared_ptr<Bus>> convoyVector;
        for (auto grp: groups){
            convoyVector.push_back(groupQueues[grp].front());
            groupQueues[grp].pop_front();
        }
        auto convoy = std::make_shared<Convoy>(totalConvoy, convoySize, convoyVector, convoyLineVector);
        nextLink->convoyEnteringLink(convoy);
        totalConvoy += 1;
    }
    convoyLineVector.clear();
}

void BusGenerator::convoyFixHeadwayDispatch(double time){
    std::vector<int> convoyLineVector;
    if (time-lastDepartureTimeConvoy >= convoyFixedHeadway) {
        std::vector<int> convoyLineVector;
        for (auto &grp: groups){
            if (!groupQueues[grp].empty()){
                convoyLineVector.push_back(groupQueues[grp].front()->busLine);
            }
            if (convoyLineVector.size() == convoySize) break;
        }
        if (convoyLineVector.size() == convoySize) { // ready to be convoyed
            std::vector<std::shared_ptr<Bus>> convoyVector;
            for (auto grp: groups){
                convoyVector.push_back(groupQueues[grp].front());
                groupQueues[grp].pop_front();
            }
            auto convoy = std::make_shared<Convoy>(totalConvoy, convoySize, convoyVector, convoyLineVector);
            nextLink->convoyEnteringLink(convoy);
            totalConvoy += 1;
            lastDepartureTimeConvoy = time;
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
    if (dispatchMode == DispatchMode::Normal || dispatchMode == DispatchMode::SerialFixHeadway) {
        for (auto grp: serialGroups){
            for (auto &bus: serialGroupQueues[grp]){
                if (bus->isPeak) {
                    bus->delayAtEachStop[-1] += 1.0;
                }
            }
        }
    }else{
        for (auto grp: groups){
            for (auto &bus: groupQueues[grp]){
                if (bus->isPeak) {
                    bus->delayAtEachStop[-1] += 1.0;
                }
            }
        }
    }
}


void BusGenerator::writeBusQueueToJson(nlohmann::json &j, double time){
    // visualization needs rewrite!!!
    
//    std::string timeString = std::to_string(time);
//    for (auto ln:lines){
//        std::string lineIDStr = std::to_string(ln);
//        if (!busQueues[ln].empty()) {
//            j["bus_queues"][lineIDStr][timeString] = int(busQueues[ln].size());
//        }else{
//            j["bus_queues"][lineIDStr][timeString] = 0;
//        }
//    }
}















