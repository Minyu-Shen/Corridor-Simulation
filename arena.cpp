//
//  arena.cpp
//  Corridor-Simulation
//
//  Created by samuel on 7/7/18.
//  Copyright © 2018 Samuel. All rights reserved.
//

#include <cmath>
#include "arena.hpp"
#include <iostream>
#include "Bus.hpp"

double max(double x, double y){
    return x > y ? x : y;
}

double min(double x, double y){
    return x < y ? x : y;
}

double cdfexp(double t, double lambda){
    return 1.0 - exp(-lambda * t);
}

double randu(){
    return double(rand())/double(RAND_MAX);
}

double randn(){
    static const double two_pi = 2 * 3.1415926;
    double u1 = randu(), u2 = randu();
    return sqrt(-2.0 * log(u1)) * cos(two_pi * u2);
}

double gammaServTime(double avg, double cs){
    std::random_device r;
    std::default_random_engine generator(r());
    double k = 1.0/cs;
    double theta = avg/k;
    std::gamma_distribution<double> distribution(k, theta);
    double res = distribution(generator);
//    std::cout << res << std::endl;
    return res;
}

void expTime(std::deque<double> &result, double hdw, double duration){
    std::random_device r;
    std::default_random_engine generator(r());
    // hdw is second/veh
    std::exponential_distribution<double> distribution(1.0/hdw);
    double period = distribution(generator);
    if (period <= duration) {
        result.push_back(period);
    }else{
        return;
    }
    while (result.back() <= duration) {
        double period = distribution(generator);
        double moment = result.back() + period;
        if (moment <= duration) result.push_back(moment);
        else break;
    }
    return;
}

//void expTimeAnlt(std::deque<double> &result, double hdw, double duration){
//    while (true) {
//        double uniNumber = randu();
//        std::cout << "uniNumber: " << uniNumber << std::endl;
//        double nextTime = -log(uniNumber) / (1/hdw);
//        std::cout << "nextTime: " << nextTime << std::endl;
//        if (result.empty()) {
//            if (nextTime <= duration) {
//                result.push_back(nextTime);
//            }
//        }else{
//            if (result.back()+nextTime <= duration) {
//                double moment = result.back() + nextTime;
//                result.push_back(moment);
//            }else{
//                break;
//            }
//        }
//    }
//}


void gaussianTimeIndepent(std::deque<double> &result, double hdw, double cv, double duration){
    int lastScheduleNo = 0;
    while (true){
        // schedule 20 one time
        for(int l = lastScheduleNo; l < lastScheduleNo+20; l++){
            double arriveTime = (l+1) * hdw + randn() * (cv*hdw);
            if (arriveTime >= duration) {
                return;
            }else{
                result.push_back(arriveTime);
            }
        }
        lastScheduleNo += 20;
    }
    return;
}

void gaussianTime(std::deque<double> &result, double hdw, double cv, double duration){
    std::random_device r;
    std::default_random_engine generator(r());
    
    // hdw is the mean, second/veh
    std::normal_distribution<double> distribution(hdw, cv*hdw);
    double period = distribution(generator);
    if (period <= duration) {
        result.push_back(period);
    }else{
        return;
    }
    while (result.back() <= duration) {
        double period = distribution(generator);
//        std::cout << period << std::endl;
        double moment = result.back() + period;
        if (moment <= duration) result.push_back(moment);
        else break;
    }
    return;
}

int binomial(int val, double prob){
    std::random_device r;
    std::default_random_engine generator(r());
    std::binomial_distribution<int> distribution(val, prob);
    return distribution(generator);
}

void copyVector(std::vector<double> &base, std::vector<double> &&inc){
    base = move(inc);
}

void subVector(std::vector<double> &base, std::vector<double> &inc){
    size_t n = base.size();
    for(size_t i = 0; i < n; ++i){
        base[i] -= inc[i];
    }
}

void multiplyVector(std::vector<double> &base, double mul){
    size_t n = base.size();
    for(size_t i = 0; i < n; ++i){
        base[i] *= mul;
    }
}

double sumVector(vd &base){
    double res = 0.0;
    for (size_t i = 0; i < base.size(); i++) {
        res += base[i];
    }
    return res;
}

void addVector(std::vector<double> &base, std::vector<double> &inc){
    size_t n = base.size();
    for(size_t i = 0; i < n; ++i){
        base[i] += inc[i];
    }
}

//std::pair<double, double> statistic(const std::vector<double> &vhts){
//    double mean = 0.0, stdvar = 0.0;
//    for(auto &x: vhts){
//        mean += x;
//        stdvar += (x*x);
//    }
//    int test_runs = (int)vhts.size();
//    mean /= double(test_runs);
//    stdvar /= double(test_runs);
//    stdvar = sqrt(stdvar - mean * mean);
//    return std::make_pair(mean, stdvar);
//}

double calVariance(const std::vector<double> &vec){
    double meanSum = 0.0, squareMeanSum = 0.0;
    for(auto &x: vec){
        meanSum += x;
        squareMeanSum += (x*x);
    }
    double size = double(int(vec.size()));
    double mean = meanSum / size;
    return  squareMeanSum/size - mean*mean;
}

int computeRuns(std::map<int, std::vector<double>> estimatingRunsMap){
//    std::vector<double> totalDelayVec;
//    for (auto &m: estimatingRunsMap){
//        double sums = sumVector(m.second);
//        totalDelayVec.push_back(sums);
//    }
//    double mean = sumVector(totalDelayVec) / (double)totalDelayVec.size();
//    double var = calVariance(totalDelayVec);
//    int nruns = int(var/(0.2*mean));
//    return nruns;
    
    std::vector<double> totalDelayVec;
    for (auto &m: estimatingRunsMap){
        double sums = sumVector(m.second);
        totalDelayVec.push_back(sums);
    }
    double mean = sumVector(totalDelayVec) / (double)totalDelayVec.size();
    double var = calVariance(totalDelayVec);
    double nruns = sqrt(var) / (mean * 0.02);
    return (int)nruns;
    
    
//    int stopSize = int(estimatingRunsMap[0].size()-1);
//    int maxRuns = 0;
//    for (int stop = 0; stop < stopSize; stop++) { // no consolidation stop, for simplicity
//        std::vector<double> eachStop;
//        for (auto &m: estimatingRunsMap){
//            eachStop.push_back(m.second[stop]);
//        }
//        double mean = sumVector(eachStop) / (double)(eachStop.size());
//        double var = calVariance(eachStop);
//        // 1.0 is the threshold
//        int nruns = int(var/(0.25*mean));
//        if (nruns > maxRuns) maxRuns = nruns;
//        eachStop.clear();
//    }
//    return maxRuns;
}

void computeMeanDelay(vd &stopDelays, vd &stopServices, std::vector<std::shared_ptr<Bus>> busPtrs){
    
    int stopSize = int(stopServices.size());
    
    // link delays are commented, for future (if needed)
    // last index (i.e. stopSize) is the consolidation stop
    for (auto d:stopDelays) d = 0.0; // initialization
    std::vector<int> stopSamples (stopSize+1);
//    std::vector<int> linkSamples (stopSize);
    
    // initialization
    for (int k = 0; k < stopSize; k++){
        stopSamples[k] = 0;
//        linkSamples[k] = 0;
    }
    stopSamples[stopSize] = 0;
    
    // collecting...
    std::vector<double> sums;
    for (auto &bus: busPtrs){
        sums.push_back(bus->delayAtEachStop[-1]);
        
        for (int s = 0; s < stopSize; s++) {
//            double linkDelay = bus->delayOnEachLink[s];
            stopDelays[s] += bus->delayAtEachStop[s];
            stopServices[s] += bus->serviceTimeAtEachStop[s];
            stopSamples[s] += 1;
//            linkDelays[s] += linkDelay;
//            linkSamples[s] += 1;
        }
        stopDelays[stopSize] += bus->delayAtEachStop[-1];
        stopSamples[stopSize] += 1;
    }
    
    
    // calculate the mean
    for (int s = 0; s < stopSize; s++) {
        if (stopSamples[s] > 0) {
            stopDelays[s] = stopDelays[s] / stopSamples[s];
            stopServices[s] = stopServices[s] / stopSamples[s];
        }else{
            stopDelays[s] = 0.0;
            stopServices[s] = 0.0;
        }
//        if (linkSamples[s] > 0) {
//            linkDelays[s] = linkDelays[s] / linkSamples[s];
//        }else{
//            linkDelays[s] = 0.0;
//        }
    }
    if (stopSamples[stopSize] > 0) {
        stopDelays[stopSize] = stopDelays[stopSize] / stopSamples[stopSize];
    }else{
        stopDelays[stopSize] = 0.0;
    }
}

void getMapFromStringFlow(std::stringstream &ss, std::map<int, double> &map){
    int number = 0;
    while(ss.good()){
        std::string substr;
        getline(ss, substr, ',');
        map.insert(std::make_pair(number, std::stod(substr)));
        number ++;
    }
}

//void computeBunchingRMSE(vd &stopRMSE, std::vector<std::shared_ptr<Bus>> busPtrs, double busFlow, double travelTime, double warmupTime){
//    int stopSize = int(stopRMSE.size());
//    double lostTime = 10.0;
//    double boardingRate = 4.0;
//    // calculate the first peak bus schedule arrival
//    double headway = 3600/busFlow; //in seconds
//    double firstScheduleArrivalTime = ceil(warmupTime/headway) * headway;
//
//    // only calculate one specific line bunching rate
//    int count = 0;
//    for (auto &bus: busPtrs){
//        if (bus->busLine == 0) {
//            for (int s = 0; s < stopSize; s++) {
//                int busRuns = bus->busID;
//                double scheduledArrivalTime = 0.0;
//                if (s == 0) {
//                    scheduledArrivalTime = firstScheduleArrivalTime + busRuns*headway + travelTime;
//                }else{
//                    scheduledArrivalTime = firstScheduleArrivalTime + busRuns*headway + s*(lostTime+boardingRate*headway+travelTime);
//                }
//                double actualArriveTime = bus->arrivalTimeEachStop[s];
//                stopRMSE[s] += (actualArriveTime-scheduledArrivalTime) * (actualArriveTime-scheduledArrivalTime);
//            }
//            count ++;
//        }
//    }
//    for (int s = 0; s < stopSize; s++) {
//        stopRMSE[s] = sqrt(stopRMSE[s] / count);
//    }
//}

void calculateBunchingRMSE(vd &stopRMSE, std::vector<std::shared_ptr<Bus>> busPtrs, double busFlow){
    // first calculate the actual arrival headway
    // i.e., h_{n,s} = a_{n,s} - a_{n-1,s}
    int stopSize = int(stopRMSE.size());
    double headway = 3600/busFlow; //in seconds
    for (int s = 0; s < stopSize; s++) {
        std::vector<double> actualArrivals;
        for (auto &bus: busPtrs){
            // only calculate one specific line's bunching
            if (bus->busLine == 0) {
                actualArrivals.push_back(bus->arrivalTimeEachStop[s]);
            }
        }
        double squareErrorSum = 0.0;
        int samples = int(actualArrivals.size());
        std::sort(actualArrivals.begin(), actualArrivals.end());
        for (int is = 0; is < samples-1; is++) {
            if (actualArrivals[is+1] == 0 || actualArrivals[is] == 0) {
                // some buses not reaching some downstream stops
                // do not count
            }else{
                squareErrorSum += pow(actualArrivals[is+1] - actualArrivals[is] - headway, 2);
            }
        }
        stopRMSE[s] = sqrt(squareErrorSum / (samples-1));
    }
    
}

void writeJsonToFile(nlohmann::json js){
    std::string s = js.dump();
    std::ofstream file;
    file.open("/Users/samuel/Desktop/corridor-animation/data.json");
    file << s;
    file.close();
}





