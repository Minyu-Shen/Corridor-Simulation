//
//  Corridor.cpp
//  Corridor-Simulation
//
//  Created by samuel on 7/27/18.
//  Copyright © 2018 samuel. All rights reserved.
//

#include "Corridor.hpp"

Corridor::Corridor(){
    
}

void Corridor::reset(){
    busGenerator->reset();
    if (!stops.empty()) {
        for (auto &stop: stops) stop->reset();
    }else{
        for (auto &stop: convoyStops) stop->reset();
    }
    for (auto &link: links) link->reset();
}

void Corridor::addBusGenerator(std::shared_ptr<BusGenerator> bg){
    busGenerator = bg;
}

void Corridor::addStop(std::shared_ptr<PaxStop> stp){
    stops.push_back(stp);
}

void Corridor::addConvoyStop(std::shared_ptr<PaxConvoyStop> stp){
    convoyStops.push_back(stp);
}

void Corridor::addLink(std::shared_ptr<Link> lk){
    links.push_back(lk);
}

void Corridor::addStaticStop(std::shared_ptr<StaticStop> stp){
    staticStops.push_back(stp);
}

void Corridor::simulate(double time){
    busGenerator->arrival(time);
    busGenerator->dispatch(time);
    if (stops.size() == 0) { // it is the convoy case
        for (int i = 0; i < (int)convoyStops.size(); i++) {
            links[i]->operationOnLink();
            convoyStops[i]->paxArrival();
            convoyStops[i]->operation();
//            links[i]->updateBusStats();
            convoyStops[i]->updateBusStats();
        }
        
    }else{ // it is the normal case
        for (int i = 0; i < (int)stops.size(); i++) {
            links[i]->operationOnLink();
            stops[i]->paxArrival();
            stops[i]->operation();
//            links[i]->updateBusStats();
            stops[i]->updateBusStats();
        }
    }
    busGenerator->updateBusStats();
    // write the states into the json object
    // after the simulation procedure
//    writeToJsonEachTime(time);
}

std::vector<double> Corridor::collectDelayAsArray(){
    std::vector<double> delayArray;
    if (stops.size() == 0) { // it is the convoy case
//        for (auto stop: stops){
//            delayArray.push_back(stop->stopDelays);
//        }
        for (auto stop: convoyStops){
//            std::cout << stop->stopDelays << std::endl;
            delayArray.push_back(stop->stopDelays/busGenerator->totlePeakBus);
        }
        delayArray.push_back(busGenerator->consolidDelays/busGenerator->totlePeakBus);
    }else{
        for (auto stop: stops){
//            if (stop->stopID == 0) {
//                std::cout << stop->stopDelays << std::endl;
//            }
            delayArray.push_back(stop->stopDelays/busGenerator->totlePeakBus);
        }
        delayArray.push_back(busGenerator->consolidDelays/busGenerator->totlePeakBus);
    }
    return delayArray;
}

void Corridor::writeToJsonEachTime(double time){
    if (stops.size() == 0) {
        if (time == 0) {
            jsObject["is_convoy"] = true;
            jsObject["line_list"] = {0, 1, 2};
        }
        for (int i = 0; i < (int)convoyStops.size(); i++) {
            links[i]->writeToJson(jsObject, time);
            convoyStops[i]->writeToJson(jsObject, time);
        }
    }else{
        if (time == 0) {
            jsObject["is_convoy"] = false;
        }
        for (int i = 0; i < (int)stops.size(); i++) {
            links[i]->writeToJson(jsObject, time);
            stops[i]->writeToJson(jsObject, time);
        }
    }
    busGenerator->writeBusQueueToJson(jsObject, time);
    
}

void Corridor::paxDemandBounding(double d){
    if (!stops.empty()) {
        for (auto &stop: stops){
            stop->paxDemandBounding(d);
        }
    }else{
        for (auto &stop: convoyStops){
            stop->paxDemandBounding(d);
        }
    }
}






