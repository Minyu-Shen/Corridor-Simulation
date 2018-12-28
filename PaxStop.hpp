//
//  PaxStop.hpp
//  Corridor-Simulation
//
//  Created by samuel on 7/26/18.
//  Copyright © 2018 samuel. All rights reserved.
//

#ifndef PaxStop_hpp
#define PaxStop_hpp

#include <vector>
#include <deque>
#include <map>
#include "arena.hpp"
#include "json.hpp"

class Bus;
class Queues;
class Link;


class PaxStop{
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
    
    // berth size
    int berthSize;
    
    // berths ptr
    std::vector<std::shared_ptr<Bus>> busesInStop;
    
    // waiting (before entering berth) zone at the bus stop
    std::deque<std::shared_ptr<Bus>> busesInWaitzone;
    
    // mark whether the bus is finished or not, true means is servicing
    // ****** index 0 means most-upstream ******
    std::vector<bool> servicingMark;

    // berth allocation plan, routeNo => berthNo
    std::map<int,int>allocationPlan;
    
    // overtaking in and out or not
    bool isOvertakeIn, isOvertakeOut;
    
    // the entering type is normal or allocation
    EnteringTypes enterType;
    
    // methods
    // constructor
    PaxStop(int sd, int bh_sz, const std::map<int, double> ldm, EnteringTypes eType, QueuingRules qRule);
    
    void reset();
    
    // add the allocation plan
    void addAllocationPlan(std::map<int,int>allocPlan);
    
    // pax arrivals
    void paxArrival();
    
    // bus arrivals, from the last link
    void busArrival(std::shared_ptr<Bus> bus);
    
    // bus operations
    void operation();
    
    // writing into the json
    void writeToJson(nlohmann::json &j, double time);
    
    // bound the pax demand for warm up period
    void paxDemandBounding(double d);
    
    // update the stats
    void updateBusStats();
    
private:
    
    void normalEntering();
    
    void allocationEntering();
    
    // enter the berth from the waitZone
    void entering();
    
    // boarding
    void paxOnOff();
    
    // bus leaving
    void leaving();
    
    // push the bus into the berth
    void pushBusToBerth(std::shared_ptr<Bus> bus, int bth_no);
    
    // check boarding is finished or not
    bool boardingAlightingCompleted(std::shared_ptr<Bus> bus);
    
    // check can leave or not (blockage)
    bool canLeave(int berthNo);
    
};
#endif /* PaxStop_hpp */
