//
//  Corridor.hpp
//  Corridor-Simulation
//
//  Created by samuel on 7/27/18.
//  Copyright © 2018 samuel. All rights reserved.
//

#ifndef Corridor_hpp
#define Corridor_hpp
#include "Link.hpp"
#include "PaxStop.hpp"
#include "PaxConvoyStop.hpp"
#include "BusGenerator.hpp"
#include "json.hpp"
#include <iostream>


class Corridor{
    
public:
    // bus stops in the corridor, not convoy type
    std::vector<std::shared_ptr<PaxStop> > stops;
    
    std::vector<std::shared_ptr<StaticStop>> staticStops;
    
    std::vector<std::shared_ptr<PaxConvoyStop>> convoyStops;
    
    // road links in the corridor
    std::vector<std::shared_ptr<Link> > links;
    
    // the bus generator at the beginning of the generator
    std::shared_ptr<BusGenerator> busGenerator;
    
    // record the total buses in the system
//    std::
    
    // constructor
    Corridor();
    
    // reset
    void reset();
    
    // add bus generator
    void addBusGenerator(std::shared_ptr<BusGenerator> bg);
    
    // add the given bus stop to the corridor
    void addStaticStop(std::shared_ptr<StaticStop> stp);
    
    // add the given bus stop to the corridor
    void addStop(std::shared_ptr<PaxStop> stp);
    
    // add the given convoy bus stop to the corridor
    void addConvoyStop(std::shared_ptr<PaxConvoyStop> stp);
    
    // add the given road link to the corridor
    void addLink(std::shared_ptr<Link> lk);
    
    // simulate one step at the given time
    void simulate(double time);

    // bound the pax demand for warm up period
    void paxDemandBounding(double d);
    
    std::vector<double> collectDelayAsArray();
    
    // json writer
    void writeToJsonEachTime (double time);
    
    // json object to record
    nlohmann::json jsObject;
    
};

#endif /* Corridor_hpp */
