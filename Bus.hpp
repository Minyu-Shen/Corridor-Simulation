//
//  Bus.hpp
//  Corridor-Simulation
//
//  Created by samuel on 7/7/18.
//  Copyright © 2018 Samuel. All rights reserved.
//

#ifndef Bus_hpp
#define Bus_hpp

#include <memory>
#include <map>
#include <vector>

class Queues;
class Bus{

public:
    
    // service time total
    double totalTimeInService;
    
    // delay time total
    double totalDelays;
    
    // enter in peak period or not?
    bool isPeak;
    
    // bus id
    int busID;
    
    
    // check if the bus arrives at that stop
    // the most import one !!!
    std::map<int, bool> isEnterEachStop;
    
    // delay stats
    std::map<int, double> entryDelayEachStop;
    std::map<int, double> exitDelayEachStop;
    
    std::map<int, double> delayAtEachStop; //-1 is the consolidation
    std::map<int, double> delayOnEachLink;
    //service time at each stop
    std::map<int, double> serviceTimeAtEachStop;
    // bus arrival time stats
    std::map<int, double> arrivalTimeEachStop;
    // remaining capacity at each stop
    std::map<int, double> paxNoEachStop;
    
    // passenger queues on the bus
    // only for common line case, currently no use.
    std::shared_ptr<Queues> paxOnBusQueue;
    
    // the bus line no
    int busLine;
    
    // the bus capacity
    double capacity;
    
    // position
    double position;
    
    // total pax number
    double kPax;
    
    // acc/dec time
    double lostTime;
    
    // boarding rate
    double boardingPaxPerDelata;
    
    // alighting rate
    double alightingPaxPerDelta;
    
    // alighting probability, the parameter of binomial distribution
    double alightProbability;
    
    // remaining static service time
//    double staticServiceTime;
    
    // average speed, determined after entering the link
    double avgSpeedOnCurrentLink;
    
    // total alight pax in each stop
    // determined when arriving at each stop
    double alightingPaxEachStop; // contradicted with "paxOnBusQueue", modify it later

//methods
    // constructor
    Bus(int id, int bl, double cpt, double bd_rt, double al_rt, double al_prob, double init_pax, int kStop);
        
    // pax board the bus, return the actual boarding No.
    double boarding(int ln, double paxOnStop);
    
    // pax alight the bus
    void alighting(int ln);
    
    // set "alightingPaxEachStop"
    void determineAlightingPaxNo(int ln);
    
    double remainSpace();
    
};

#endif /* Bus_hpp */
