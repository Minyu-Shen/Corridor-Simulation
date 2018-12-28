//
//  Link.hpp
//  Corridor-Simulation
//
//  Created by samuel on 7/7/18.
//  Copyright © 2018 Samuel. All rights reserved.
//

#ifndef Link_hpp
#define Link_hpp

#include <vector>
#include <deque>
#include "json.hpp"

class Bus;
class Convoy;
class PaxConvoyStop;
class PaxStop;
class StaticStop;

class Link{
    
public:
    
    // link id
    int linkID;
    
    // link length
    double linkLength;
    
    // check if conovy or bus
    bool isConvoy;
    
    // the buses on the roadway
    std::deque<std::shared_ptr<Bus>> busesOnLink;
    
    // the convoys on the roadway
    std::vector<std::shared_ptr<Convoy>> convoysOnLink;
    
    // mean and c.v. of travel time, for gaussian
    double travelTimeMean, travelTimeStd;
    
    // convoy penalty factor
    double convoyPenaltyFactor;
    
    // next pax convoy stop
    std::shared_ptr<PaxConvoyStop> nextConovyStop;
    
    // next pax stop
    std::shared_ptr<PaxStop> nextStop;
    
    void updateBusStats();
    
// methods:
    
    // constructor
    Link(int id, double ttMean, double ttStd, bool isC);
    
    // reset function
    void reset();
    
    // buses entering the link
    void busEnteringLink(std::shared_ptr<Bus> &bus);
    
    // convoys entering the link
    void convoyEnteringLink(std::shared_ptr<Convoy> & convoy);
    
    // bus operations on the link
    void operationOnLink();
    
    // writing into the json
    void writeToJson(nlohmann::json &j, double time);
    
private:
    
    // forward the buses
    void forward();
    
    // bus leaving the link, to the waitZone (point queue)
    void leaving();
    
};


#endif /* Link_hpp */
