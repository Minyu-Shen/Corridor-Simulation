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

PaxConvoyStop::PaxConvoyStop(int sd, int bh_sz, const std::map<int, double> ldm){
    stopID = sd; berthSize = bh_sz;
    nextLink = nullptr;
    // ldm: line -> arrival demand mapping
    paxQueues = std::make_shared<Queues>(ldm);
    for (auto &m: ldm){
        destinations.push_back(m.first);
    }
//    std::fill(servicingMark.begin(), servicingMark.end(), false);
    convoyInStop = nullptr;
    simTimeNow = 0.0;
    stopDelays = 0.0;
}

void PaxConvoyStop::reset(){
    simTimeNow = 0.0;
    paxQueues->reset();
    convoyInStop = nullptr;
    convoysInWaitzone.clear();
    stopDelays = 0.0;
//    std::fill(servicingMark.begin(), servicingMark.end(), false);
}

// 0. pax arrivals
void PaxConvoyStop::paxArrival(){
    paxQueues->arrival();
}

// 1. convoy arrivals
void PaxConvoyStop::convoyArrival(std::shared_ptr<Convoy> convoy){
    // add to the waitZone first, and then check if it can proceed to the berth
    for (auto &bus: convoy->buses){
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
        convoyInStop->buses[c]->lostTime = 0.0 + reactionTime*(berthSize-c-1);
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
                // boarding ...
                double paxOnStop = paxQueues->query(ln);
                if (paxOnStop > 0){ // the stop has line "ln" pax
                    double actualBoardPax = bus->boarding(ln, paxOnStop);
                    paxQueues->decrease(ln, actualBoardPax);
                }
                
                // alighting ...
                bus->alighting(ln);
                
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
    if (paxQueues->query(bus->busLine) > 0 && bus->remainSpace() > 0) return false;
    return true;
}

bool PaxConvoyStop::boardingCompleted(){
    bool isFinished = true;
    for (auto &bus: convoyInStop->buses){
        int ln = bus->busLine;
        if (paxQueues->query(ln) > 0){
            if (bus->remainSpace() > 0.0) {
                isFinished = false;
                break;
            }
        }
        
        if (bus->alightingPaxEachStop > 0) {
            isFinished = false;
            break;
        }
    }
    return isFinished;
}

void PaxConvoyStop::operation(){
    entering();
    boarding();
    leaving();
    simTimeNow += 1.0;
}

void PaxConvoyStop::paxDemandBounding(double d){
    paxQueues->paxDemandBounding(d);
}

void PaxConvoyStop::updateBusStats(){
//    if (convoyInStop != nullptr) {
//        for (auto &bus: convoyInStop->buses){
//            bus->delayAtEachStop[stopID] += 1.0;
//        }
//    }
//    for (auto &convoy: convoysInWaitzone){
//        for (auto &bus: convoy->buses){
//            bus->delayAtEachStop[stopID] += 1.0;
//        }
//    }

    // **********
    
    
//    int blockCount = 0;
//    for (int i = int(convoysInWaitzone.size())-1; i >= 0; i--) {
//        auto convoy = convoysInWaitzone[i];
//        int peakCount = 0;
//        for (auto &bus: convoy->buses){
//            if (bus->isPeak) {
//                blockCount ++;
//                peakCount ++;
//            }
//        }
//        if (peakCount == convoy->convoySize) break;
//    }
//    stopDelays += blockCount * 1.0;
    
    
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
    for (auto des:destinations){
        double pax = paxQueues->query(des);
        std::string lineIDStr = std::to_string(des);
        std::string stopIDStr = std::to_string(stopID);
        j["pax_queues"][lineIDStr][timeString][stopIDStr] = pax;
    }
}
