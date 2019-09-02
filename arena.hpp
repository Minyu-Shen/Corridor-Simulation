//
//  arena.hpp
//  Corridor-Simulation
//
//  Created by samuel on 7/7/18.
//  Copyright © 2018 Samuel. All rights reserved.
//

#ifndef arena_hpp
#define arena_hpp

#include <random>
#include <vector>
#include <deque>
#include "json.hpp"
#include <fstream>

class Bus;

enum class EnteringTypes{Normal, Allocation};
enum class QueuingRules{Parallel, FIFO, LimitedOvertaking, FreeOvertaking, FreeOvertakingWithBlock};

typedef std::vector<double> vd;

// cdf function of exponential distribution (lambda)
double cdfexp(double t, double lambda);

double max(double x, double y);

double min(double x, double y);

// random sample uniform between 0 and 1
double randu();

// random sample normal distribution N(0, 1)
double randn();

double gammaServTime(double avg, double cs);

void expTime(std::deque<double> &result, double hdw, double duration);

//void expTimeAnlt(std::deque<double> &result, double hdw, double duration);

void gaussianTime(std::deque<double> &result, double hdw, double cv, double duration);

void gaussianTimeIndepent(std::deque<double> &result, double hdw, double cv, double offset, double duration);

int binomial(int val, double prob);

// copy 'inc' vector onto 'base' vector
void copyVector(vd &base, vd &&inc);

// subtract 'inc' vector from 'base' vector
void subVector(vd &base, vd &inc);

// element-wise multiply 'base' vector with scalar 'mul'
void multiplyVector(vd &base, double mul);

// return the sum of one vector
double sumVector(vd &base);

// add 'inc' vector onto 'base' vector
void addVector(vd &base, vd &inc);

// calculate mean and std of an array
//std::pair<double, double> statistic(const std::vector<double> &vhts);

// calculate the mean of an array
double calMean(const std::vector<double> &vec);

// calculate the variance of an array
double calVariance(const std::vector<double> &vec);

// calculate number runs needed
int computeRuns(std::map<int, std::vector<double>> estimatingRunsMap);

void computeMeanDelay(vd &stopDelays, vd &stopDelayCvs, vd &meanDwellTimes, vd &cvDwellTimes, vd &stopEntryDelays, vd &stopExitDelays, vd &stopPaxNos, std::vector<std::shared_ptr<Bus>> busPtrs);

void getMapFromStringFlow(std::stringstream &ss, std::map<int, double> &map);

// calculate the bunching RMSE (wrong version~)
//void computeBunchingRMSE(vd &stopRMSE, std::vector<std::shared_ptr<Bus>> busPtrs, double busFlow, double travelTime, double warmupTime);

//void calculateBunchingRMSE(vd &stopRMSE, vd &stopDepartureRMSE, std::vector<std::shared_ptr<Bus>> busPtrs, double busFlow);

void calculateHeadwayVariation(int kLine, vd &arrivalHeadwayMean, vd &arrivalHeadwayCv, vd &entryHeadwayMean, vd &entryHeadwayCv, vd &departureHeadwayMean, vd &departureHeadwayCv, std::vector<std::shared_ptr<Bus>> busPtrs);

std::pair<double, double> calHeadwayStatsFromTimes(vd &times);

void writeJsonToFile(nlohmann::json js);
#endif /* arena_hpp */
