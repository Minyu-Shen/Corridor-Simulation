//
//  PaxStop.cpp
//  Corridor-Simulation
//
//  Created by samuel on 7/26/18.
//  Copyright © 2018 samuel. All rights reserved.
//

#include "PaxStop.hpp"
#include "Queues.hpp"
#include <iostream>
#include "Bus.hpp"
#include "Link.hpp"

PaxStop::PaxStop(int sd, int bh_sz, const std::map<int, double> ldm, EnteringTypes eType, QueuingRules qRule, double cp_ratio, double cp_ratio_all, const std::map<int, int> lineGroupAMap, int serialGSize){
    
    stopID = sd; berthSize = bh_sz; enterType = eType;
    common_ratio = cp_ratio; common_ratio_all = cp_ratio_all;
    nextLink = nullptr; lineGroupAssignMap = lineGroupAMap;
    busesInStop.resize(bh_sz); servicingMark.resize(bh_sz);
    std::fill(servicingMark.begin(), servicingMark.end(), false);
    std::fill(busesInStop.begin(), busesInStop.end(), nullptr);
    
    L = (int)ldm.size();
//    groupLineSize = L / bh_sz; // "m" in the paper
    groupLineSize = L / serialGSize; // "m" in the paper
    
    // first get one line demand
    double oneLineDemand = ldm.find(0)->second;
    // 1. common pax queue for all the lines ...
    std::map<int, double>commanAllDemandMap;
    commanAllDemandMap[-1] = oneLineDemand * L * common_ratio_all; // use -1 as index for all the lines
    commonAllPaxQueue = std::make_shared<Queues>(commanAllDemandMap);
    
    double remainTotalDemand = oneLineDemand * L * (1-common_ratio_all);
    double remainCommonDemandForOneGroup = remainTotalDemand * common_ratio / serialGSize;
    double remainUncommonDemandForOneLineInGroup = remainTotalDemand * (1-common_ratio) / L;
    // 2. (unshared) uncommon pax demand, for each line ...
    std::map<int, double>uncommonDemandMap; // line -> lambda
    for (auto &map: ldm){
        lines.push_back(map.first);
        uncommonDemandMap[map.first] = remainUncommonDemandForOneLineInGroup;
    }
    uncommonPaxQueues = std::make_shared<Queues>(uncommonDemandMap);
    
    // 3. create common pax demand, for each group ...
    std::map<int, double>commonDemandMap; // group -> lambda, group size = serial group size
    for (int m_it = 0; m_it < serialGSize; m_it++) {
        commonDemandMap[m_it] = remainCommonDemandForOneGroup;
    }
    commonPaxQueue = std::make_shared<Queues>(commonDemandMap);
    /*
    // 2. (unshared) uncommon pax demand, for each line ...
    
    std::map<int, double>uncommonDemandMap; // line -> lambda
    for (auto &map: ldm){
        lines.push_back(map.first);
        uncommonDemandMap[map.first] = oneLineDemand * (1-common_ratio_all) * (1-common_ratio);
    }
    uncommonPaxQueues = std::make_shared<Queues>(uncommonDemandMap);
     
     // 3. create common pax demand, for each group ...
     std::map<int, double>commonDemandMap; // group -> lambda, group size = serial group size
     for (int m_it = 0; m_it < serialGSize; m_it++) {
     commonDemandMap[m_it] = oneLineDemand * (1-common_ratio_all) * common_ratio * groupLineSize;
     }
    */

    
    
    std::fill(servicingMark.begin(), servicingMark.end(), false);
    
    if (qRule == QueuingRules::FIFO || qRule == QueuingRules::LimitedOvertaking) isOvertakeIn = false;
    else isOvertakeIn = true;
    
    if (qRule == QueuingRules::LimitedOvertaking || qRule == QueuingRules::FreeOvertaking || qRule == QueuingRules::FreeOvertakingWithBlock) isOvertakeOut = true;
    else isOvertakeOut = false;
    
    if (qRule == QueuingRules::Parallel){
        isParallel = true;
//        busesInParallel = std::set<std::shared_ptr<Bus>>{};
    } else {
        isParallel = false;
    }
    
    stopDelays = 0.0;
    simTimeNow = 0.0;
}

void PaxStop::reset(){
    simTimeNow = 0.0;
    uncommonPaxQueues->reset();
    commonPaxQueue->reset();
    commonAllPaxQueue->reset();
    busesInWaitzone.clear();
    if (isParallel) {
        busesInParallel.clear();
    } else {
        std::fill(busesInStop.begin(), busesInStop.end(), nullptr);
        std::fill(servicingMark.begin(), servicingMark.end(), false);
    }
    stopDelays = 0.0;
}


// for the allocation case
void PaxStop::addAllocationPlan(std::map<int, int> allocPlan){
    if (enterType == EnteringTypes::Allocation) {
        // initialze allocationPlan
        for(auto &m: allocPlan){
            allocationPlan.insert(std::make_pair(m.first, m.second));
        }
    }else{
        std::cout << "Something wrong, stop type is not allocation" << std::endl;
    }
}

// 0 and 1 is called in the main function

// 0. pax arrivals
void PaxStop::paxArrival(){
    commonAllPaxQueue->arrival();
    uncommonPaxQueues->arrival();
    commonPaxQueue->arrival();
}

// 1. buses arrivals, from last link
void PaxStop::busArrival(std::shared_ptr<Bus> bus){
    // record the bus has arrived at this stop
    bus->isEnterEachStop[stopID] = true;
    // record the current pax no.
    bus->paxNoEachStop[stopID] = bus->kPax;
    // record the bus arrival time here
    bus->arrivalTimeEachStop[stopID] = simTimeNow;
    // set the "alightingPaxEachStop"
    bus->determineAlightingPaxNo(bus->busLine);
    
    
    if (bus->remainSpace() <= 0) { // directly leave stop
        if (nextLink == nullptr) { // finally finished!
        }else{
            nextLink->busEnteringLink(bus);
        }
        bus->departureTimeEachStop[stopID] = simTimeNow;
    }else{
        // first push bus into waitZone temporarily
        busesInWaitzone.push_back(bus);
    }
    
    /* may be needed in the future !!!
     
    // check whether need to alight
    if (bus->alightingPaxEachStop > 0) { // some pax want to alight
        // first put bus in the waitZone
        // then at same simulation delta, check if it can proceed to stop
        busesInWaitzone.push_back(bus);
        
    } else { // no pax wants to alight
        // check if there is a same line serving or waiting in the stop
        bool isFindSameLine = false;
        for (auto &busInStop: busesInStop){
            if (busInStop == nullptr) continue;
            if (busInStop->busLine == bus->busLine){
                isFindSameLine = true;
                break;
            }
        }
        if (!isFindSameLine) {
            for (auto &busInWaitZone: busesInWaitzone){
                if (busInWaitZone->busLine == bus->busLine) {
                    isFindSameLine = true;
                    break;
                }
            }
        }
        if (isFindSameLine) { // leave the stop directly
            if (nextLink == nullptr) { // finally finished!
//                std::cout << bus->busID << " bus's leaving corridor : " << simTimeNow << "by skipping the last stop" << std::endl;
            } else{
                nextLink->busEnteringLink(bus);
            }
        }else{
            busesInWaitzone.push_back(bus); // enter the stop tamely -, -
        }
    }
     */
}

// 2. buses entering
void PaxStop::entering(){
    if (!busesInWaitzone.empty()) {
        if (enterType == EnteringTypes::Normal) normalEntering();
        else if(enterType == EnteringTypes::Allocation) allocationEntering();
        else std::cout << "Entering type is wrong" << std::endl;
    }
}

void PaxStop::normalEntering(){
    while (!busesInWaitzone.empty()) {
        auto bus = busesInWaitzone.front();
        if (isParallel) {
            bus->lostTime = 8.0;
            busesInParallel.insert(bus);
            busesInWaitzone.pop_front();
        } else {
            if (isOvertakeIn) {
                // just check the most-downstream empty berth
                int mostDownStreamBerthEmpty = -1;
                for (int c = berthSize-1; c >= 0; c--) {
                    if (servicingMark[c] == false){ // as long as found, break;
                        mostDownStreamBerthEmpty = c;
                        break;
                    }
                }
                if (mostDownStreamBerthEmpty >= 0) { // the berths are not all serving
                    pushBusToBerth(bus, mostDownStreamBerthEmpty);
                }else{
                    // no empty berth, break the while
                    break;
                }
            } else{
                // check if can enter without overtake-in
                int berthAvailable = -1;
                for (int c = 0; c < berthSize; c++) {
                    if (servicingMark[c] == true) break;
                    else berthAvailable = c;
                }
                if (berthAvailable >= 0) { // at least one berth is empty
                    pushBusToBerth(bus, berthAvailable);
                }else{
                    // no empty berth, break the while
                    break;
                }
            }
        }
    }
}

void PaxStop::allocationEntering(){
    while (!busesInWaitzone.empty()) {
        auto bus = busesInWaitzone.front();
        int targetBerthNo = allocationPlan[bus->busLine];
        if (!servicingMark[targetBerthNo]){ // the target berth is empty
            bool isUpstreamBlockded = false;
            for (int c=0; c<targetBerthNo; c++) {
                if (servicingMark[c] == true){
                    isUpstreamBlockded = true; break;
                }
            }
            if (isUpstreamBlockded) { // find true, i.e., upstream berths are not all empty
                if (isOvertakeIn) {
                    // push the bus into the berth first, and then count the lost time
                    pushBusToBerth(bus, targetBerthNo);
                    std::cout << bus->busID << "entering time:" << simTimeNow << std::endl;
                }else{
                    break; // wait at waiting zone, do nothing
                }
            }else{ // can reach the target berth freely
                pushBusToBerth(bus, targetBerthNo);
                std::cout << bus->busID << "entering time:" << simTimeNow << std::endl;
            }
        }else{ // the target berth is full
            break; // wait at waiting zone, do nothing
        }
    }
}


void PaxStop::pushBusToBerth(std::shared_ptr<Bus> bus, int bth_no){
    bus->entryTimeEachStop[stopID] = simTimeNow;
    bus->lostTime = 8.0;
    busesInStop[bth_no] = bus;
    busesInWaitzone.pop_front();
    servicingMark[bth_no] = true;
}

// 3. pax boarding and alighting
void PaxStop::paxOnOff(){
    if (isParallel) {
        for (auto bus: busesInParallel){
            oneBusOnOff(bus);
        }
    } else {
        // loop the busesInStop to board
        for (auto bus: busesInStop){
            if (bus == nullptr) continue;
            oneBusOnOff(bus);
        }
    }
}

void PaxStop::oneBusOnOff(std::shared_ptr<Bus> bus){
    if (bus->lostTime <= 0.0) { // acc/dec finished
        int ln = bus->busLine;
        // alighting ...
        bus->alighting(ln);
        
        // boarding ...
        int group = lineGroupAssignMap[ln];
        double surplus_board = 100; // unbounded for the first time
        
        if (surplus_board > 0) {
            // 1. check common pax for all lines
            double commonAllPaxOnStop = commonAllPaxQueue->query(-1);
            if (commonAllPaxOnStop > 0) {
                double actualCommonAllPaxBoard = bus->boarding(-1, commonAllPaxOnStop, surplus_board);
                commonAllPaxQueue->decrease(-1, actualCommonAllPaxBoard);
            }
        }
        if (surplus_board > 0) {
            // 2. check common pax within group
            double commonPaxOnStop = commonPaxQueue->query(group);
            if (commonPaxOnStop > 0) {
                double actualCommonPaxBoard = bus->boarding(group, commonPaxOnStop, surplus_board);
                commonPaxQueue->decrease(group, actualCommonPaxBoard);
            }
        }
        
        if (surplus_board > 0) {
            // 3. check uncommon pax
            double uncommonPaxOnStop = uncommonPaxQueues->query(ln);
            if (uncommonPaxOnStop > 0) {
                double actualUnCommonPaxBoard = bus->boarding(ln, uncommonPaxOnStop, surplus_board);
                uncommonPaxQueues->decrease(ln, actualUnCommonPaxBoard);
            }
        }
        
        
//        int group = lineGroupAssignMap[ln];
//        double commonPaxOnStop = commonPaxQueue->query(group);
//        double uncommonPaxOnStop = uncommonPaxQueues->query(ln);
//        if (commonPaxOnStop > 0) {
//            if (uncommonPaxOnStop > 0) {
//                int rd_value = rand() % (2);
//                if (rd_value == 0) { // first load common
//                    double surplus_board = 100; // unbounded for the first time
//                    double actualCommonPaxBoard = bus->boarding(group, commonPaxOnStop, surplus_board);
//                    commonPaxQueue->decrease(group, actualCommonPaxBoard);
//                    if (surplus_board > 0) {
//                        double actualUnCommonPaxBoard = bus->boarding(ln, uncommonPaxOnStop, surplus_board);
//                        uncommonPaxQueues->decrease(ln, actualUnCommonPaxBoard);
//                    }
//                }else{ // first load uncommon
//                    double surplus_board = 100; // unbounded for the first time
//                    double actualUnCommonPaxBoard = bus->boarding(ln, uncommonPaxOnStop, surplus_board);
//                    commonPaxQueue->decrease(group, actualUnCommonPaxBoard);
//                    if (surplus_board > 0) {
//                        double actualCommonPaxBoard = bus->boarding(group, commonPaxOnStop, surplus_board);
//                        uncommonPaxQueues->decrease(ln, actualCommonPaxBoard);
//                    }
//                }
//            }else{ // only common
//                double surplus_board = 100; // unbounded for the first time
//                double actualCommonPaxBoard = bus->boarding(group, commonPaxOnStop, surplus_board);
//                commonPaxQueue->decrease(group, actualCommonPaxBoard);
//            }
//        }else{
//            if (uncommonPaxOnStop > 0) { // only uncommon
//                double surplus_board = 100; // unbounded for the first time
//                double actualUnCommonPaxBoard = bus->boarding(ln, uncommonPaxOnStop, surplus_board);
//                uncommonPaxQueues->decrease(ln, actualUnCommonPaxBoard);
//            }else{ // no any pax
//                // do nothing
//            }
//        }
        
    } else{ // still acc/dec
        bus->lostTime -= 1.0;
    }
}


// 4. buses leaving
void PaxStop::leaving(){
    if (isParallel) {
        std::set<std::shared_ptr<Bus>>::iterator it;
        for (it = busesInParallel.begin(); it != busesInParallel.end(); ) {
            std::shared_ptr<Bus> bus = *it;
            if (boardingAlightingCompleted(bus)) {
                bus->departureTimeEachStop[stopID] = simTimeNow;
                if (nextLink == nullptr) { // finally finished!
                    
                } else{
                    nextLink->busEnteringLink(bus);
                }
                busesInParallel.erase(it++);
            }else{
                ++it;
            }
        }

    } else {
        for (int i = int(busesInStop.size())-1; i >= 0; i--) {
            if (busesInStop[i] == nullptr) continue;
            if (boardingAlightingCompleted(busesInStop[i])) {
                if (canLeave(i)) {
                    busesInStop[i]->departureTimeEachStop[stopID] = simTimeNow;
                    if (nextLink == nullptr) { // finally finished!
                        
                    } else{
                        nextLink->busEnteringLink(busesInStop[i]);
                    }
                    busesInStop[i] = nullptr;
                    servicingMark[i] = false;
                }else{
                    // just wait
                }
            }
        }
    }
}

bool PaxStop::canLeave(int berthNo){
    if (isOvertakeOut) return true;
    if (berthNo == berthSize-1) {
        return true;
    }else{
        // check if all the downstream berths are empty
        for (int c = berthNo+1; c < berthSize; c++) {
            if (servicingMark[c] == true) return false;
        }
        return true;
    }
}

bool PaxStop::boardingAlightingCompleted(std::shared_ptr<Bus> bus){
    if (bus->alightingPaxEachStop > 0) return false;
    int ln = bus->busLine;
    // check if the all lines' common queue is empty
    if (commonAllPaxQueue->query(-1) > 0 && bus->remainSpace() > 0) return false;
    // check if this unique uncommon line have pax
    if (uncommonPaxQueues->query(ln) > 0 && bus->remainSpace() > 0) return false;
    // check if the other common lines have pax
    int group = lineGroupAssignMap[ln];
    if (commonPaxQueue->query(group) > 0 && bus->remainSpace() > 0) return false;
    
//    for (auto ln_it:lines){
//        if (lineGroupAssignMap[ln_it] == group) {
//            if (commonPaxQueue->query(ln_it) > 0 && bus->remainSpace() > 0) return false;
//        }
//    }
    return true;
}

void PaxStop::operation(){
    entering();
    paxOnOff();
    leaving();
    simTimeNow += 1.0;
}

void PaxStop::paxDemandBounding(double d){
    commonAllPaxQueue->paxDemandBounding(d * L * common_ratio_all);
    commonPaxQueue->paxDemandBounding(d * (1-common_ratio_all) * common_ratio * groupLineSize);
    uncommonPaxQueues->paxDemandBounding(d * (1-common_ratio_all) * (1-common_ratio));
}

// for stats
void PaxStop::updateBusStats(){
    if (isParallel) {
        for (auto bus : busesInParallel) {
            if (bus->isPeak) {
                bus->serviceTimeAtEachStop[stopID] += 1.0;
            }
        }
    } else {
        for (int i = 0; i < int(busesInStop.size()); i++) {
            auto bus = busesInStop[i];
            if (bus == nullptr) continue;
            if (bus->isPeak) {
                if (boardingAlightingCompleted(bus)) {
                    if (!canLeave(i)) {
                        bus->delayAtEachStop[stopID] += 1.0;
                        bus->exitDelayEachStop[stopID] += 1.0;
                    }
                }else{ // still serving, for recording effective service time
                    bus->serviceTimeAtEachStop[stopID] += 1.0;
                }
            }
        }
        for (int i = int(busesInWaitzone.size())-1; i >= 0; i--) {
            auto bus = busesInWaitzone[i];
            if (bus->isPeak) {
                bus->delayAtEachStop[stopID] += 1.0;
                bus->entryDelayEachStop[stopID] += 1.0;
            }
        }
    }
}

void PaxStop::writeToJson(nlohmann::json &j, double time){
    std::string timeString = std::to_string(time);
    
    for (int c = 0; c < busesInStop.size(); c++) {
        auto bus = busesInStop[c];
        if (bus == nullptr) continue;
        std::string busIDStr = std::to_string(bus->busID);
        j["buses"][busIDStr][timeString]["link_id"] = -1; // means not on the link
        j["buses"][busIDStr][timeString]["stop_id"] = stopID;
        j["buses"][busIDStr][timeString]["berth_id"] = c;
        if (!boardingAlightingCompleted(bus)) {
            j["buses"][busIDStr][timeString]["is_serving"] = true;
        }else{
            j["buses"][busIDStr][timeString]["is_serving"] = false;
        }
        j["buses"][busIDStr][timeString]["line_no"] = bus->busLine;
    }
    
    // write the buses in the waitZone into the json
    for (int c = 0; c < busesInWaitzone.size(); c++) {
        auto bus = busesInWaitzone[c];
        if (bus == nullptr) continue;
        std::string busIDStr = std::to_string(bus->busID);
        j["buses"][busIDStr][timeString]["link_id"] = -1; // means not on the link
        j["buses"][busIDStr][timeString]["stop_id"] = stopID;
        j["buses"][busIDStr][timeString]["berth_id"] = -1; // means not in the berth
        j["buses"][busIDStr][timeString]["wait_position"] = c;
        j["buses"][busIDStr][timeString]["line_no"] = bus->busLine;
    }
    
    // write the paxqueues of each stop into the json
    for (auto des:lines){
        double pax = uncommonPaxQueues->query(des);
//        std::string lineIndicator = std::to_string("line");
        std::string lineIDStr = std::to_string(des);
        std::string stopIDStr = std::to_string(stopID);
        j["pax_queues"][lineIDStr][timeString][stopIDStr] = pax;
    }
}





