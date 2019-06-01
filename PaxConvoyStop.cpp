//
//  PaxConvoyStop.cpp
//  Corridor-Simulation
//
//  Created by samuel on 7/22/18.
//  Copyright © 2018 samuel. All rights reserved.
//

#include "PaxConvoyStop.hpp"
#include "Queues.hpp"
#include "Link.hpp"
#include "Convoy.hpp"
#include <iostream>

PaxConvoyStop::PaxConvoyStop(int sd, int bh_sz, const std::map<int, double> ldm, double cp_ratio, double cp_ratio_all, const std::map<int, int> lineGroupAMap){
    stopID = sd; berthSize = bh_sz; lineGroupAssignMap = lineGroupAMap;
    nextLink = nullptr; common_ratio = cp_ratio; common_ratio_all = cp_ratio_all;
    
    int L = (int)ldm.size();
    groupLineSize = L / bh_sz;
    
    // (unshared) uncommon pax demand, for each line
    std::map<int, double>uncommonDemandMap; // line -> lambda
    double oneLineDemand = 0.0;
    for (auto &map: ldm){
        lines.push_back(map.first);
        uncommonDemandMap[map.first] = map.second * (1-common_ratio);
        if (map.first == 0) oneLineDemand = map.second;
    }
    // create common pax demand, for each group
    std::map<int, double>commonDemandMap; // group -> lambda
    for (int m_it = 0; m_it < bh_sz; m_it++) {
        commonDemandMap[m_it] = oneLineDemand * common_ratio * groupLineSize;
    }
    commonPaxQueue = std::make_shared<Queues>(commonDemandMap);
    uncommonPaxQueues = std::make_shared<Queues>(uncommonDemandMap);
    
    convoyInStop = nullptr;
    simTimeNow = 0.0;
    stopDelays = 0.0;
}

void PaxConvoyStop::reset(){
    simTimeNow = 0.0;
    commonPaxQueue->reset();
    uncommonPaxQueues->reset();
    convoyInStop = nullptr;
    convoysInWaitzone.clear();
    stopDelays = 0.0;
//    std::fill(servicingMark.begin(), servicingMark.end(), false);
}

// 0. pax arrivals
void PaxConvoyStop::paxArrival(){
    commonPaxQueue->arrival();
    uncommonPaxQueues->arrival();
}

// 1. convoy arrivals
void PaxConvoyStop::convoyArrival(std::shared_ptr<Convoy> convoy){
    // add to the waitZone first, and then check if it can proceed to the berth
    for (auto &bus: convoy->buses){
        // record the bus has arrived at this stop
        bus->isEnterEachStop[stopID] = true;
        
        // record the remaining capacity for each stop
        bus->paxNoEachStop[stopID] = bus->kPax;
        // record the arrival time of buses (all are equal)
        bus->arrivalTimeEachStop[stopID] = simTimeNow;
        bus->determineAlightingPaxNo(bus->busLine);
    }
    
    if (!convoysInWaitzone.empty()) {
        convoysInWaitzone.push_back(convoy);
    }else{ // the waitZone is empty, directly proceed to the berth
        convoysInWaitzone.push_back(convoy);
    }
}

// 2. convoy entering
void PaxConvoyStop::entering(){
    if (!convoysInWaitzone.empty()) {
        if (convoyInStop == nullptr) {
            auto convoy = convoysInWaitzone.front();
            pushConvoyToBerth(convoy);
        }
        
        // check if the berth is all empty
//        bool isAllEmpty = true;
//        for (auto mark: servicingMark){
//            if (mark){
//                isAllEmpty = false; break;
//            }
//        }
//        if (isAllEmpty) {
//            auto convoy = convoysInWaitzone.front();
//            pushConvoyToBerth(convoy);
//        }
        
    }
}

void PaxConvoyStop::pushConvoyToBerth(std::shared_ptr<Convoy> convoy){
//    std::fill(servicingMark.begin(), servicingMark.end(), true);
    convoyInStop = convoy;
    convoysInWaitzone.pop_front();
    double reactionTime = 0; // for future ...
    for (int c=0; c<berthSize; c++) {
        convoyInStop->buses[c]->lostTime = 8.0 + reactionTime*(berthSize-c-1);
    }
}

// 3. convoy boarding
void PaxConvoyStop::boarding(){
    // loop the convoy can serve
    if (convoyInStop != nullptr) {
        for (int c=0; c<convoyInStop->convoySize; c++) {
            int ln = convoyInStop->lines[c];
            auto bus = convoyInStop->buses[c];
            if (bus->lostTime <= 0.0) { // acc/dec finished
                // alighting ...
                bus->alighting(ln);
                // boarding ...
                int group = lineGroupAssignMap[ln];
                double commonPaxOnStop = commonPaxQueue->query(group);
                double uncommonPaxOnStop = uncommonPaxQueues->query(ln);
                if (commonPaxOnStop > 0) {
                    if (uncommonPaxOnStop > 0) {
                        int rd_value = rand() % (2);
                        if (rd_value == 0) { // first load common
                            double surplus_board = 100; // unbounded for the first time
                            double actualCommonPaxBoard = bus->boarding(group, commonPaxOnStop, surplus_board);
                            commonPaxQueue->decrease(group, actualCommonPaxBoard);
                            if (surplus_board > 0) {
                                double actualUnCommonPaxBoard = bus->boarding(ln, uncommonPaxOnStop, surplus_board);
                                uncommonPaxQueues->decrease(ln, actualUnCommonPaxBoard);
                            }
                        }else{ // first load uncommon
                            double surplus_board = 100; // unbounded for the first time
                            double actualUnCommonPaxBoard = bus->boarding(ln, uncommonPaxOnStop, surplus_board);
                            commonPaxQueue->decrease(group, actualUnCommonPaxBoard);
                            if (surplus_board > 0) {
                                double actualCommonPaxBoard = bus->boarding(group, commonPaxOnStop, surplus_board);
                                uncommonPaxQueues->decrease(ln, actualCommonPaxBoard);
                            }
                        }
                    }else{ // only common
                        double surplus_board = 100; // unbounded for the first time
                        double actualCommonPaxBoard = bus->boarding(group, commonPaxOnStop, surplus_board);
                        commonPaxQueue->decrease(group, actualCommonPaxBoard);
                    }
                }else{
                    if (uncommonPaxOnStop > 0) { // only uncommon
                        double surplus_board = 100; // unbounded for the first time
                        double actualUnCommonPaxBoard = bus->boarding(ln, uncommonPaxOnStop, surplus_board);
                        uncommonPaxQueues->decrease(ln, actualUnCommonPaxBoard);
                    }else{ // no any pax
                        // do nothing
                    }
                }
                
            }else{
                bus->lostTime -= 1.0;
            }
        }
    }
}

// 4. convoy leaving
void PaxConvoyStop::leaving(){
    if (convoyInStop != nullptr) {
        if (boardingCompleted()) {
            for (auto &bus: convoyInStop->buses){
                bus->departureTimeEachStop[stopID] = simTimeNow;
            }
            if (nextLink == nullptr) { // finally finished!
            } else{
                nextLink->convoyEnteringLink(convoyInStop);
            }
            convoyInStop = nullptr;
        }
    }
}

bool PaxConvoyStop::boardingAlightingCompleted(std::shared_ptr<Bus> bus){
    if (bus->alightingPaxEachStop > 0) return false;
    int ln = bus->busLine;
    // check if this unique uncommon line have pax
    if (uncommonPaxQueues->query(ln) > 0 && bus->remainSpace() > 0) return false;
    // check if the other common lines have pax
    int group = lineGroupAssignMap[ln];
    if (commonPaxQueue->query(group) > 0 && bus->remainSpace() > 0) return false;
    return true;
}

bool PaxConvoyStop::boardingCompleted(){
    for (auto &bus: convoyInStop->buses){
        if (bus->alightingPaxEachStop > 0) return false;
        int ln = bus->busLine;
        // check if this unique uncommon line have pax
        if (uncommonPaxQueues->query(ln) > 0 && bus->remainSpace() > 0) return false;
        // check if the other common lines have pax
        int group = lineGroupAssignMap[ln];
        if (commonPaxQueue->query(group) > 0 && bus->remainSpace() > 0) return false;
    }
    return true;
    
//        bool isFinished = true;
//        if (paxQueues->query(ln) > 0){
//            if (bus->remainSpace() > 0.0) {
//                isFinished = false;
//                break;
//            }
//        }
//        if (bus->alightingPaxEachStop > 0) {
//            isFinished = false;
//            break;
//        }
//    }
//    return isFinished;
}

void PaxConvoyStop::operation(){
    entering();
    boarding();
    leaving();
    simTimeNow += 1.0;
}

void PaxConvoyStop::paxDemandBounding(double d){
    commonPaxQueue->paxDemandBounding(common_ratio * d * groupLineSize);
    uncommonPaxQueues->paxDemandBounding((1-common_ratio) * d);
}

void PaxConvoyStop::updateBusStats(){

    // entry delays
    for (auto convoy: convoysInWaitzone){
        for (auto &bus: convoy->buses){
            if (bus->isPeak) {
                bus->delayAtEachStop[stopID] += 1.0;
                bus->entryDelayEachStop[stopID] += 1.0;
            }
        }
    }
    // "exiting" (wait for other buses in the convoy) delays
    if (convoyInStop != nullptr) {
        for (auto &bus: convoyInStop->buses){
            if (bus->isPeak) {
                if (boardingAlightingCompleted(bus)) {
                    bus->exitDelayEachStop[stopID] += 1.0;
                    bus->delayAtEachStop[stopID] += 1.0;
                }else{
                    bus->serviceTimeAtEachStop[stopID] += 1.0;
                }
            }
        }
    }
}

void PaxConvoyStop::writeToJson(nlohmann::json &j, double time){
    std::string timeString = std::to_string(time);
    
    if (convoyInStop != nullptr) {
        std::string convoyIDString = std::to_string(convoyInStop->convoyID);
        // write the buses in the stop into the json
        j["convoys"][convoyIDString][timeString]["stop_id"] = stopID;
        j["convoys"][convoyIDString][timeString]["link_id"] = -1; // means not on the link
        j["convoys"][convoyIDString][timeString]["is_serving"] = true;
    }
    
    // write the buses in the waitZone into the json
    for (int c = 0; c < convoysInWaitzone.size(); c++){
        std::string convoyIDString = std::to_string(convoysInWaitzone[c]->convoyID);
        j["convoys"][convoyIDString][timeString]["link_id"] = -1;
        j["convoys"][convoyIDString][timeString]["stop_id"] = stopID;
        j["convoys"][convoyIDString][timeString]["is_serving"] = false;
        j["convoys"][convoyIDString][timeString]["wait_position"] = c;
    }

    // write the paxqueues of each stop into the json
    for (auto des:lines){
        double pax = paxQueues->query(des);
        std::string lineIDStr = std::to_string(des);
        std::string stopIDStr = std::to_string(stopID);
        j["pax_queues"][lineIDStr][timeString][stopIDStr] = pax;
    }
}
