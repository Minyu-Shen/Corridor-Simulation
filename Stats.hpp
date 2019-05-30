//
//  Stats.hpp
//  Corridor
//
//  Created by Samuel on 5/30/19.
//  Copyright © 2019 samuel. All rights reserved.
//

#ifndef Stats_hpp
#define Stats_hpp
#include <vector>

class Stats{
public:

    // delays at each stop
    std::vector<double> totalDelay;
    std::vector<double> meanService;
    std::vector<double> bunchingRMSE;
    std::vector<double> entryDelay;
    std::vector<double> exitDelay;
    std::vector<double> paxNo;
    
    // constructor
    Stats(int objective, int stopSize);
    
    
    // method:
    // update stats
//    void update(const std::vector<double> thisRound);
};

#endif /* Stats_hpp */
