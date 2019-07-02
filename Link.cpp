//
//  Link.cpp
//  Corridor-Simulation
//
//  Created by samuel on 7/7/18.
//  Copyright © 2018 Samuel. All rights reserved.
//

#include "Link.hpp"
#include "Convoy.hpp"
#include "arena.hpp"
#include "PaxConvoyStop.hpp"
#include <iostream>
#include "Bus.hpp"
#include "PaxStop.hpp"

Link::Link(int id, double ttMean, double ttStd, bool isC){
    linkID = id;
    convoyPenaltyFactor = 1.0; isConvoy = isC;
    travelTimeMean = ttMean; travelTimeStd = ttStd;
    linkLength = 1000.0; // meters
}

void Link::reset(){
    busesOnLink.clear();
    convoysOnLink.clear();
}

void Link::busEnteringLink(std::shared_ptr<Bus> &bus){
    double tt = max(0.0, travelTimeMean+randn()*travelTimeStd);
//    std::cout << tt << std::endl;
    bus->avgSpeedOnCurrentLink = linkLength / tt;
    bus->position = 0.0;
    busesOnLink.push_back(bus);
//    std::cout << bus->busID << " bus's travel time on Link " << linkID << " : " << tt << std::endl;
}

void Link::convoyEnteringLink(std::shared_ptr<Convoy> &convoy){
    double tt = max(0.0, travelTimeMean+randn()*travelTimeStd);
    convoy->avgSpeedOnCurrentLink = linkLength / (convoyPenaltyFactor*tt);
    convoy->position = 0.0;
    convoysOnLink.push_back(convoy);
//    std::cout << convoy->convoyID << " convoy's travel time on Link " << linkID << " : " << tt << std::endl;
}

// 1. forward the buses
void Link::forward(){
    if (isConvoy) {
        for (auto &convoy: convoysOnLink){
            // the speed is m/s
            convoy->position += convoy->avgSpeedOnCurrentLink * 1.0;
        }
    } else{ // loop all the buses
        for (auto &bus: busesOnLink){
            // the speed is m/s
            bus->position += bus->avgSpeedOnCurrentLink * 1.0;
        }
    }
}

// 2. leaving
void Link::leaving(){
    if (isConvoy) {
        // sorting, let the most-downstream convoy (with position) to be the back()
        static auto cmp = [](const std::shared_ptr<Convoy> &a, const std::shared_ptr<Convoy> &b) {
            return a->position < b->position;
        };
        std::sort(convoysOnLink.begin(), convoysOnLink.end(), cmp);
        
        while (convoysOnLink.size() > 0){
            if (convoysOnLink.back()->position >= linkLength) {
                if (nextConovyStop != nullptr) {
                    nextConovyStop->convoyArrival(convoysOnLink.back());
//                    std::cout << convoysOnLink.back()->convoyID <<" next stop ID: " << nextConovyStop->stopID << std::endl;
                    // delete the convoy from the vector
                    convoysOnLink.pop_back();
                }else{
                    std::cout << "something wrong, no stop after link" << std::endl;
                    // just delete the convoy from the vector
                    convoysOnLink.pop_back();
                }
            } else {
                break;
            }
        }
    }else{ // normal leaving
        // sorting, let the most-downstream bus (with position) to be the back()
        static auto cmp = [](const std::shared_ptr<Bus> &a, const std::shared_ptr<Bus> &b) {
            return a->position < b->position;
        };
        std::sort(busesOnLink.begin(), busesOnLink.end(), cmp);
        
        while (busesOnLink.size() > 0) {
            if (busesOnLink.back()->position >= linkLength) {
                if (nextStop != nullptr) {
                    nextStop->busArrival(busesOnLink.back());
                    busesOnLink.pop_back();
                } else{
                    busesOnLink.pop_back();
                }
            } else{
                break;
            }
        }
    }
}

// bus operation
void Link::operationOnLink(){
    if (isConvoy) {
        if (convoysOnLink.size() > 0) {
            forward();
            leaving();
        }
    } else{
        if (busesOnLink.size() > 0) {
            forward();
            leaving();
        }
    }
}

void Link::updateBusStats(){
    if (isConvoy) {
        if (convoysOnLink.size() > 0) {
            for (auto &convoy: convoysOnLink){
                for (auto &bus: convoy->buses){
                    bus->delayOnEachLink[linkID] += 1.0;
                }
            }
        }
    } else{
        if (busesOnLink.size() > 0) {
            for (auto &bus: busesOnLink){
                bus->delayOnEachLink[linkID] += 1.0;
            }
        }
    }
}


// pass the josn, and update it
void Link::writeToJson(nlohmann::json &j, double time){
    std::string timeString = std::to_string(time);
    if (isConvoy) {
        for (auto &convoy: convoysOnLink){
            std::string idString = std::to_string(convoy->convoyID);
            j["convoys"][idString][timeString]["link_id"] = linkID;
            j["convoys"][idString][timeString]["position"] = convoy->position;
            j["convoys"][idString][timeString]["stop_id"] = -1; // means not in the stop
        }
    }else{
        for (auto &bus: busesOnLink){
            std::string idString = std::to_string(bus->busID);
            j["buses"][idString][timeString]["link_id"] = linkID;
            j["buses"][idString][timeString]["position"] = bus->position;
            j["buses"][idString][timeString]["stop_id"] = -1; // means not in the stop
            j["buses"][idString][timeString]["line_no"] = bus->busLine;
        }
    }
}



