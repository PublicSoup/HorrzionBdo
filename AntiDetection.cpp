#include "AntiDetection.h"
#include <iostream>

AntiDetection::AntiDetection() 
    : rng(std::chrono::steady_clock::now().time_since_epoch().count()),
      timingDist(50, 200),  // Default timing range 50-200ms
      pauseDist(1, 100),    // For pause chance calculation
      jitterDist(-5.0, 5.0), // Default jitter range -5 to +5 pixels
      minDelay(50),
      maxDelay(200),
      pauseChance(10),      // 10% chance of random pause
      maxJitter(5.0) {
}

AntiDetection::~AntiDetection() {
}

void AntiDetection::RandomizeTiming() {
    // Add a random delay based on configured timing range
    int delay = timingDist(rng);
    std::this_thread::sleep_for(std::chrono::milliseconds(delay));
}

void AntiDetection::AddMouseJitter() {
    // Get current mouse position
    POINT currentPos;
    if (GetCursorPos(&currentPos)) {
        // Calculate jitter offset
        double xJitter = jitterDist(rng);
        double yJitter = jitterDist(rng);
        
        // Apply jitter to mouse position
        int newX = currentPos.x + static_cast<int>(xJitter);
        int newY = currentPos.y + static_cast<int>(yJitter);
        
        // Set new mouse position
        SetCursorPos(newX, newY);
        
        // Small delay to make the movement look more natural
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        
        // Optionally move back (uncomment if needed)
        // SetCursorPos(currentPos.x, currentPos.y);
    }
}

bool AntiDetection::ShouldTakeRandomPause() {
    // Generate random number and check against pause chance
    int randomValue = pauseDist(rng);
    return randomValue <= pauseChance;
}

void AntiDetection::SetTimingRange(int min, int max) {
    if (min > 0 && max > min) {
        minDelay = min;
        maxDelay = max;
        timingDist = std::uniform_int_distribution<int>(min, max);
    }
}

void AntiDetection::SetPauseChance(int chance) {
    if (chance >= 0 && chance <= 100) {
        pauseChance = chance;
    }
}

void AntiDetection::SetJitterRange(double maxJitterValue) {
    if (maxJitterValue > 0) {
        maxJitter = maxJitterValue;
        jitterDist = std::uniform_real_distribution<double>(-maxJitter, maxJitter);
    }
}

void AntiDetection::RandomDelay(int minMs, int maxMs) {
    if (minMs > 0 && maxMs > minMs) {
        std::uniform_int_distribution<int> delayDist(minMs, maxMs);
        int delay = delayDist(rng);
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    }
}