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
#include <set>
#include <iterator>
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
    
    // lines
    std::vector<int> lines;
    
    // pointer for the next link
    std::shared_ptr<Link> nextLink;
    
    // pax queues
    std::shared_ptr<Queues>uncommonPaxQueues;
    
    // common pax queues: groupNo -> common pax No.
    std::shared_ptr<Queues>commonPaxQueue;
    
    // line->group plan
    std::map<int, int>lineGroupAssignMap;
    
    // cp ratio within group
    double common_ratio;
    
    // cp ratio for all lines
    double common_ratio_all;
    
    // berth size
    int berthSize;
    
    // berths ptr
    std::vector<std::shared_ptr<Bus>> busesInStop;
    
    // unlimited parallel berths
    std::set<std::shared_ptr<Bus>> busesInParallel;
    
    // waiting (before entering berth) zone at the bus stop
    std::deque<std::shared_ptr<Bus>> busesInWaitzone;
    
    // mark whether the bus is finished or not, true means is servicing
    // ****** index 0 means most-upstream ******
    std::vector<bool> servicingMark;

    // berth allocation plan, routeNo => berthNo
    std::map<int,int>allocationPlan;
    
    // overtaking in and out or not
    bool isOvertakeIn, isOvertakeOut;
    
    // is parallel or not
    bool isParallel;
    
    // the entering type is normal or allocation
    EnteringTypes enterType;
    
    // common pax ratio for each group of line
    double cp_ratio;
    
    // group's line size (i.e., m)
    int groupLineSize;
    
    // methods
    // constructor
    PaxStop(int sd, int bh_sz, const std::map<int, double> ldm, EnteringTypes eType, QueuingRules qRule, double cp_ratio, double cp_ratio_all, const std::map<int, int> lineGroupAMap);
    
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
    
    // one bus on and off
    void oneBusOnOff(std::shared_ptr<Bus> bus);
    
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
