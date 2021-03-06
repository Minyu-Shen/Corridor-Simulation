//
//  BusGenerator.hpp
//  Corridor-Simulation
//
//  Created by samuel on 7/11/18.
//  Copyright © 2018 samuel. All rights reserved.
//

#ifndef BusGenerator_hpp
#define BusGenerator_hpp

#include <map>
#include <set>
#include <vector>
#include <deque>
#include "Bus.hpp"
#include "json.hpp"

class Queues;
class Link;
class StaticStop;
class PaxStop;
class Bus;

enum class BusArriveCorridorMode{ Poisson, Gaussian };
enum class DispatchMode{SingleNormal, Normal, Convoy, Serial, SerialFixHeadway, ConvoyFixHeadway};

class BusGenerator{
    
public:
    
    // recording total peak bus
    int totlePeakBus;
    
    // recording the delays
    double consolidDelays;
    
    // manage the buses that enter in the peak period
    std::vector<std::shared_ptr<Bus>> peakBusVec;
    
//    double simTime; // for test
    double warmupTime;
    
    // indicate whether strategy to be used
    int strategy;
    
    // maintaining consolidation queues for each group (convoy), for dispatch strategy-0 and -2
    // group No. -> deque
    std::map<int, std::deque<std::shared_ptr<Bus>>> groupQueues;
    
    // maintaining consolidation queues for each group (serial), for dispatch strategy-0 and -2
    // group No. -> deque
    std::map<int, std::deque<std::shared_ptr<Bus>>> serialGroupQueues;
    
    // line-group assignment plan (convoy)
    // line No. -> group No.
    std::map<int, int>lineGroupConvoyMap;
    
    // line-group assignment plan (serial)
    // line No. -> group No.
    std::map<int, int>lineGroupSerialMap;
    
    // bus arrival mode
    BusArriveCorridorMode arriveMode;
    
    // dispatch mode
    DispatchMode dispatchMode;
    
    // routes ids
    std::vector<int> lines;
    
    // group ids
    std::vector<int> groups;
    
    // serial group ids
    std::vector<int> serialGroups;
    
    // stop No.
    int kStop;
    
    // bus arrival mean headway for each line
    // the unit is sec/veh, if poisson arrival, it is 1/lambda
    std::map<int,double> lineMeanHeadway;
    
    // bus arrival cv headway for each line
    std::map<int,double> lineCvHeadway;
    
    // scheduled bus arrival for each route (randomness added)
    std::map<int, std::deque<double>> busSchedule;
    
    // total bus count
    int totalBus;
    
    // next link
    std::shared_ptr<Link> nextLink;
    
    // convoy size if any
    int convoySize;
    
    // total convoy cout
    int totalConvoy;
    
    // bus alight/board rate
    double boardingRate, alightingRate;
    
    // bus capacity, al_prob
    double capacity, alightingProb;
    
    // initial pax number
    double initialPax;
    
    /******** strategy -0 starts ***********/
    
    // record the last dispatch group
    int lastDispatchGroup;
    
    // record the last departure time of each  group of single bus
    // group No. -> time
    std::map<int, double> lastDepartureTimeSerialGroupMap;
    
    // record the last dispatch conovy time
    double lastDepartureTimeConvoy;
    
    /******** strategy -0 ends ***********/
    
    // totally how many convoys?
    int m;

    // fixed line headway to be set
    double lineFixedHeadway;
    
    // fixed convoy headway to be set
    double convoyFixedHeadway;
    
    // write the bus queue into the json
    void writeBusQueueToJson(nlohmann::json &j, double time);
    
//methods
    //constructor
    BusGenerator(std::map<int,double> lineMH, std::map<int,double> lineCH, BusArriveCorridorMode arrMode, DispatchMode disMode, double bd_rt, double al_rt, double cpt, double al_prob, double init_pax, int stop_no, const std::map<int, int>lineGroupCMap, const std::map<int, int>lineGroupSMap, int serialGroupSize, int convoyGroupSize, int strategy=0);
    
    void reset();
    
    void schedule(double warmupTime, double peakTime);
    
    void dispatch(double time);
    
    void arrival(double time);
    
    void convoyDispatch(double time);
    
    void convoyFixHeadwayDispatch(double time);
    
    void normalDispatch(double time);
    
    void serialDispatch(double time);
    
    void serialFixHeadwayDispatch(double time);
    
    void dispatchOneToLink(int which);// which is line or group
    
    void updateBusStats();
    
};

#endif /* BusGenerator_hpp */
