//
//  Convoy.hpp
//  Corridor-Simulation
//
//  Created by samuel on 7/9/18.
//  Copyright © 2018 samuel. All rights reserved.
//

#ifndef Convoy_hpp
#define Convoy_hpp

#include <stdio.h>
#include <vector>
#include "Bus.hpp"

class Convoy{
public:
    // convoy id
    int convoyID;
    
    // convoy size
    int convoySize;
    
    // position on the road
    double position;
    
    // conovy composition
    std::vector<std::shared_ptr<Bus>> buses;
    
    // index 0 means the most-upstream line No.
    std::vector<int> lines;
    
    // average speed, determined after entering the link
    double avgSpeedOnCurrentLink;
    
//method
    
    // constructor
    Convoy(int cID, int sz, std::vector<std::shared_ptr<Bus>> bs, std::vector<int> ls);
    
    // initialize the travel time
    void setRoadTravelTime();
    
};

#endif /* Convoy_hpp */
