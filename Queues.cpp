//
//  Queues.cpp
//  Corridor-Simulation
//
//  Created by Samuel (adapted from Wei Ni) on 7/7/18.
//  Copyright © 2018 Wei Ni. All rights reserved.
//

#include "Queues.hpp"
#include "arena.hpp"
#include <iostream>

Queues::Queues(const std::map<int,double> dms, ArrivalMode arrMode, double gcv){
    arrivalMode = arrMode; gaussianCV = gcv;
    kItem = 0.0; demandBound = 1.0e8;
    for(auto &p: dms){
        lengths.insert(std::make_pair(p.first, 0.0));
        demands.insert(std::make_pair(p.first, p.second));
        waiting.insert(std::make_pair(p.first, 0.0));
    }
}

void Queues::arrival(){
    if(arrivalMode == ArrivalMode::Poisson)
        poissonArrival();
    else if (arrivalMode == ArrivalMode::Constant) constantArrival();
    else gaussianArrival();
}

void Queues::poissonArrival(){
    double prob2 = 0.0, prob1 = 0.0, prob = 0.0, ps = 0.0;
    for(auto &p: demands){
        // demands map, first is int, second is double
        // ps is the arrival rate, buses/sec
        ps = min(p.second, demandBound);
//        std::cout << ps << std::endl;
        // ps is the lambda for each line
        if (ps == 0) {
            
        }else{
            prob2 = cdfexp(waiting[p.first]+1.0, ps);
            prob1 = cdfexp(waiting[p.first], ps);
            prob = (prob2-prob1) / (1.0-prob1);
            if(randu() < prob){  // randu() is a 0-1 uniform R.V.
                increase(p.first, 1.0);
                waiting[p.first] = 0.0;
            }
            else waiting[p.first] += 1.0;
        }
    }
}

void Queues::constantArrival(){
    for(auto &p: demands){
        // demands map, first is int, second is double
        // ps is the arrival rate, buses/sec
        double ps = min(p.second, demandBound);
//        std::cout << ps << std::endl;
        // ps is the lambda for each line
        increase(p.first, ps);
    }
}

void Queues::gaussianArrival(){
    // future ...
    
//    double prob2 = 0.0, prob1 = 0.0, prob = 0.0, ps = 0.0;
//    for(auto &p: demands){
//        ps = min(p.second, demandBound);
//        prob2 = cdfgaussian(waiting[p.first]+1.0, 1.0/ps, rev_sigma);
//        prob1 = cdfgaussian(waiting[p.first], 1.0/ps, rev_sigma);
//        prob = (prob2 - prob1) / (1.0 - prob1);
//        if(randu() < prob){
//            increase(p.first, 1.0);
//            waiting[p.first] = 0.0;
//        }
//        else waiting[p.first] += 1.0;
//    }
}

void Queues::paxDemandBounding(double d){
    demandBound = d;
}

void Queues::reset(){
    for(auto &p: demands){
        lengths[p.first] = 0.0;
        waiting[p.first] = 0.0;
    }
    kItem = 0.0;
}

bool Queues::containKey(int key) {
    return demands.find(key) != demands.end();
}

void Queues::increase(int id, double x){
    if (lengths.find(id) != lengths.end()) {
        lengths[id] += x;
        kItem += x;
    }
}

void Queues::decrease(int id, double x){
    if(lengths.find(id) != lengths.end()){
        double delta = std::min<double>(lengths[id], x);
        lengths[id] -= delta;
        kItem -= delta;
    }
}

double Queues::query(int id){
    if(lengths.find(id) != lengths.end()){
        return lengths[id];
    } else return -1;
}

double Queues::numItem() {
    return kItem;
}
