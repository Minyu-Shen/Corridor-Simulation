//
//  Queues.hpp
//  Corridor-Simulation
//
//  Created by Samuel (adapted from Wei Ni) on 7/7/18.
//  Copyright © 2018 Wei Ni. All rights reserved.
//

#ifndef Queues_hpp
#define Queues_hpp

# include <map>

enum class ArrivalMode {Constant, Poisson, Gaussian };

class Queues{
    
private:
    
    // num of item in a queue, indexed by id
    std::map<int,double> lengths;
    
    // arrival rate of a queue, indexed by id
    std::map<int,double> demands;
    
    // elapsed time after last arrival of a queue, indexed by id
    std::map<int,double> waiting;
    
    // total number of item in the queue
    double kItem;
    
    // upperbound of arrival rate
    double demandBound;
    
    // the arrival mode into the queue
    ArrivalMode arrivalMode;
    
    // gaussian arrival c.v.
    double gaussianCV;
    
    void poissonArrival();
    void gaussianArrival();
    void constantArrival();
    
public:
    
    // constructor
    Queues(const std::map<int, double> dms, ArrivalMode arrMode = ArrivalMode::Poisson, double gcv=0.3);
    
    // add x items to the queue indexed by id
    void increase(int id, double x);
    
    // remove x items from the queue indexed by id
    void decrease(int id, double x);
    
    // item entering queues (arrival process)
    void arrival();
    
    // query the num of items in the queue indexed by id
    double query(int id);
    
    // return total number of items in all queues
    double numItem();
    
    // check if there is a queue indexed by key
    bool containKey(int key);
    
    // reset all the states of these queues
    void reset();
    
    // bound the pax demand for warm up period
    void paxDemandBounding(double d);
    
};

#endif /* Queues_hpp */
