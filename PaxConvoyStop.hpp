//
//  PaxConvoyStop.hpp
//  Corridor-Simulation
//
//  Created by samuel on 7/22/18.
//  Copyright © 2018 samuel. All rights reserved.
//

#ifndef PaxConvoyStop_hpp
#define PaxConvoyStop_hpp

#include <vector>
#include <deque>
#include <map>
#include "arena.hpp"

class Convoy;
class Queues;
class Link;
class Bus;

class PaxConvoyStop{
public:
    
    // recording bus delay
    double stopDelays;
    
    // stop ID
    int stopID;
    
    // simulation time now, for testing
    double simTimeNow;
    
    // destinations
    std::vector<int> destinations;
    
    // pointer for the next link
    std::shared_ptr<Link> nextLink;
    
    // pax queues
    std::shared_ptr<Queues>paxQueues;
    
    // lines
    std::vector<int> lines;
    
    // uncommon pax queues
    std::shared_ptr<Queues>uncommonPaxQueues;
    
    // common pax queues: groupNo -> common pax No.
    std::shared_ptr<Queues>commonPaxQueue;
    
    // line->group plan
    std::map<int, int>lineGroupAssignMap;
    
    // cp ratio within group
    double common_ratio;
    
    // cp ratio for all lines
    double common_ratio_all;
    
    // group's line size (i.e., m)
    int groupLineSize;
    
    // berth size
    int berthSize;
    
    // convoy ptr
    std::shared_ptr<Convoy> convoyInStop;

    // waiting (before entering berth) zone at the bus stop
    std::deque<std::shared_ptr<Convoy>> convoysInWaitzone;
    
    // mark whether the bus is finished or not, true means is servicing
    // ****** index 0 means most-upstream ******
    std::vector<bool> servicingMark;
    
    // boarding person per simulation time
    double boardingPaxPerDelata;
    
    // methods
    
    // constructor
    PaxConvoyStop(int sd, int bh_sz, const std::map<int, double> ldm, double cp_ratio, double cp_ratio_all, const std::map<int, int> lineGroupAMap);
    
    // reset func
    void reset();
    
    // pax arrivals
    void paxArrival();
    
    // bus arrivals, from the last link
    void convoyArrival(std::shared_ptr<Convoy> convoy);
    
    // bus operations
    void operation();
    
    // bound the pax demand for warm up period
    void paxDemandBounding(double d);
    
    // update the stats
    void updateBusStats();
    
    // write the states into the json
    void writeToJson(nlohmann::json &j, double time);
    
private:
    
    // enter the berth from the waitZone
    void entering();
    
    // boarding
    void boarding();
    
    // bus leaving
    void leaving();
    
    // push the bus into the berth
    void pushConvoyToBerth(std::shared_ptr<Convoy>  convoy);
    
    bool boardingAlightingCompleted(std::shared_ptr<Bus> bus);
    
    // check boarding is finished or not
    bool boardingCompleted();
    
    
};

#endif /* PaxConvoyStop_hpp */
