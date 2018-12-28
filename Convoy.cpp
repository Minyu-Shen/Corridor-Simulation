//
//  Convoy.cpp
//  Corridor-Simulation
//
//  Created by samuel on 7/9/18.
//  Copyright © 2018 samuel. All rights reserved.
//

#include "Convoy.hpp"

Convoy::Convoy(int cID, int sz, std::vector<std::shared_ptr<Bus>> bs, std::vector<int> ls){
    convoyID = cID; convoySize = sz;
    for (auto &b: bs){
        buses.push_back(b);
    }
    for (auto l: ls){
        lines.push_back(l);
    }
}



