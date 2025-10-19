#ifndef ANTIDETECTION_H
#define ANTIDETECTION_H

#include "Platform.h"
#include <random>
#include <chrono>
#include <thread>

class AntiDetection {
private:
    std::mt19937 rng;
    std::uniform_int_distribution<int> timingDist;
    std::uniform_int_distribution<int> pauseDist;
    std::uniform_real_distribution<double> jitterDist;
    
    // Configuration parameters
    int minDelay;
    int maxDelay;
    int pauseChance;
    double maxJitter;
    
public:
    AntiDetection();
    ~AntiDetection();
    
    // Randomize timing between actions
    void RandomizeTiming();
    
    // Add random mouse movement jitter
    void AddMouseJitter();
    
    // Check if a random pause should be taken
    bool ShouldTakeRandomPause();
    
    // Set configuration parameters
    void SetTimingRange(int min, int max);
    void SetPauseChance(int chance);
    void SetJitterRange(double maxJitter);
    
    // Utility function for random delays
    void RandomDelay(int minMs, int maxMs);
};

#endif // ANTIDETECTION_H