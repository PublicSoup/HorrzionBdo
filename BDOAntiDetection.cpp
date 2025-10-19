#include "BDOAntiDetection.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <map>

// Constructor
BDOAntiDetection::BDOAntiDetection() 
    : rng(std::chrono::steady_clock::now().time_since_epoch().count()),
      reactionTimeDist(250.0, 75.0),  // Normal distribution: mean 250ms, std dev 75ms
      jitterDist(-3.0, 3.0),
      pauseDist(1, 100),
      mistakeDist(3),  // Average 3 mistakes per 100 actions
      fatigueLevel(0.0),
      skillLevel(0.5),  // Medium skill level by default
      consecutiveActions(0),
      sessionStart(std::chrono::steady_clock::now()),
      bdoWindow(nullptr),
      currentMethod(InputMethod::POST_MESSAGE) {
    
    actionHistory.clear();
}

// Destructor
BDOAntiDetection::~BDOAntiDetection() {
    // Clean up if needed
}

// Initialize and find BDO window
bool BDOAntiDetection::Initialize() {
    return FindBDOWindow();
}

bool BDOAntiDetection::FindBDOWindow() {
    // Find Black Desert window
    bdoWindow = FindWindowA(BDO::WINDOW_CLASS, nullptr);
    if (!bdoWindow) {
        bdoWindow = FindWindowA(nullptr, BDO::WINDOW_TITLE);
    }
    
    if (bdoWindow) {
        std::cout << "Black Desert window found: " << std::hex << bdoWindow << std::dec << std::endl;
        
        // Bring window to foreground for testing
        SetForegroundWindow(bdoWindow);
        return true;
    }
    
    std::cout << "Black Desert window not found. Running in simulation mode." << std::endl;
    return false;
}

// Add human-like delay based on action type
void BDOAntiDetection::AddHumanDelay(BDO::ActionType action) {
    int baseDelay = CalculateReactionTime(action);
    
    // Add fatigue factor
    double fatigueMultiplier = 1.0 + (fatigueLevel * 0.5);  // Up to 50% slower when tired
    baseDelay = static_cast<int>(baseDelay * fatigueMultiplier);
    
    // Add skill level factor (higher skill = more consistent, lower = more variation)
    double skillVariation = (1.0 - skillLevel) * 0.3;  // Up to 30% variation for beginners
    std::uniform_real_distribution<double> skillDist(1.0 - skillVariation, 1.0 + skillVariation);
    baseDelay = static_cast<int>(baseDelay * skillDist(rng));
    
    // Apply the delay
    std::this_thread::sleep_for(std::chrono::milliseconds(baseDelay));
    
    // Record the action
    RecordAction(action, baseDelay);
    
    // Update fatigue
    UpdateFatigue();
}

// Simulate fatigue over time
void BDOAntiDetection::SimulateFatigue() {
    double sessionHours = GetSessionDuration();
    
    // Fatigue increases exponentially over time
    // After 1 hour: ~10% fatigue, 2 hours: ~25%, 3 hours: ~45%, 4 hours: ~65%
    fatigueLevel = 1.0 - exp(-sessionHours * 0.4);
    fatigueLevel = std::min(fatigueLevel, 0.9);  // Cap at 90% fatigue
    
    // Add random fluctuations
    std::normal_distribution<double> fatigueDist(0.0, 0.05);
    fatigueLevel += fatigueDist(rng);
    fatigueLevel = std::max(0.0, std::min(1.0, fatigueLevel));
}

// Add micro pauses that humans naturally make
void BDOAntiDetection::AddMicroPause() {
    std::uniform_int_distribution<int> microDist(5, 50);
    std::this_thread::sleep_for(std::chrono::milliseconds(microDist(rng)));
}

// Move mouse using Bezier curves for natural movement
void BDOAntiDetection::MoveMouseBezier(int targetX, int targetY, int duration) {
    POINT currentPos;
    GetCursorPos(&currentPos);
    
    if (duration == 0) {
        // Calculate duration based on distance
        double distance = sqrt(pow(targetX - currentPos.x, 2) + pow(targetY - currentPos.y, 2));
        duration = static_cast<int>(distance * 2 + 100);  // ~2ms per pixel + 100ms base
    }
    
    // Generate control points for Bezier curve
    std::uniform_real_distribution<double> controlDist(-0.3, 0.3);
    
    POINT start = currentPos;
    POINT end = {targetX, targetY};
    
    // Create control points that deviate from straight line
    POINT control1, control2;
    control1.x = start.x + static_cast<LONG>((end.x - start.x) * 0.25 + (end.x - start.x) * controlDist(rng));
    control1.y = start.y + static_cast<LONG>((end.y - start.y) * 0.25 + (end.y - start.y) * controlDist(rng));
    
    control2.x = start.x + static_cast<LONG>((end.x - start.x) * 0.75 + (end.x - start.x) * controlDist(rng));
    control2.y = start.y + static_cast<LONG>((end.y - start.y) * 0.75 + (end.y - start.y) * controlDist(rng));
    
    // Move along the curve
    int steps = duration / 10;  // Update every 10ms
    for (int i = 0; i <= steps; i++) {
        double t = static_cast<double>(i) / steps;
        
        // Add slight acceleration/deceleration (ease-in-out)
        t = t * t * (3.0 - 2.0 * t);
        
        POINT pos = GetBezierPoint(start, control1, control2, end, t);
        
        // Add small noise to each point
        if (i > 0 && i < steps) {
            pos.x += static_cast<LONG>(jitterDist(rng) * 0.5);
            pos.y += static_cast<LONG>(jitterDist(rng) * 0.5);
        }
        
        SetCursorPos(pos.x, pos.y);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    // Ensure we end exactly at target
    SetCursorPos(targetX, targetY);
}

// Human-like mouse movement with overshoot and correction
void BDOAntiDetection::MoveMouseHuman(int targetX, int targetY) {
    POINT currentPos;
    GetCursorPos(&currentPos);
    
    double distance = sqrt(pow(targetX - currentPos.x, 2) + pow(targetY - currentPos.y, 2));
    
    // Sometimes overshoot the target (more likely for longer distances)
    std::uniform_real_distribution<double> overshootChance(0.0, 1.0);
    if (distance > 100 && overshootChance(rng) < 0.3) {
        // Overshoot by 5-15%
        std::uniform_real_distribution<double> overshootDist(1.05, 1.15);
        double overshoot = overshootDist(rng);
        
        int overshootX = currentPos.x + static_cast<int>((targetX - currentPos.x) * overshoot);
        int overshootY = currentPos.y + static_cast<int>((targetY - currentPos.y) * overshoot);
        
        // Move to overshoot position
        MoveMouseBezier(overshootX, overshootY, 0);
        
        // Small pause to "realize" the mistake
        std::this_thread::sleep_for(std::chrono::milliseconds(50 + pauseDist(rng)));
        
        // Correct to actual target
        MoveMouseBezier(targetX, targetY, 100);
    } else {
        // Normal movement
        MoveMouseBezier(targetX, targetY, 0);
    }
}

// Add random mouse noise/tremor
void BDOAntiDetection::AddMouseNoise(int intensity) {
    POINT pos;
    GetCursorPos(&pos);
    
    std::uniform_int_distribution<int> noiseDist(-intensity, intensity);
    pos.x += noiseDist(rng);
    pos.y += noiseDist(rng);
    
    SetCursorPos(pos.x, pos.y);
}

// Simulate keyboard press with human characteristics
void BDOAntiDetection::KeyPress(WORD vkCode, bool addVariation) {
    if (addVariation) {
        // Random delay before pressing
        AddMicroPause();
    }
    
    // Determine press duration (humans don't press instantly)
    std::uniform_int_distribution<int> pressDuration(30, 120);
    int duration = pressDuration(rng);
    
    if (bdoWindow && currentMethod == InputMethod::POST_MESSAGE) {
        SendKeyViaPostMessage(vkCode, true);
        std::this_thread::sleep_for(std::chrono::milliseconds(duration));
        SendKeyViaPostMessage(vkCode, false);
    } else {
        // Fallback to SendInput
        INPUT input = {0};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = vkCode;
        
        SendInput(1, &input, sizeof(INPUT));
        std::this_thread::sleep_for(std::chrono::milliseconds(duration));
        
        input.ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(1, &input, sizeof(INPUT));
    }
    
    if (addVariation) {
        // Random delay after releasing
        AddMicroPause();
    }
}

// Hold a key for specified duration
void BDOAntiDetection::KeyHold(WORD vkCode, int duration) {
    // Add human variation to hold duration
    std::uniform_real_distribution<double> holdVariation(0.9, 1.1);
    duration = static_cast<int>(duration * holdVariation(rng));
    
    if (bdoWindow && currentMethod == InputMethod::POST_MESSAGE) {
        SendKeyViaPostMessage(vkCode, true);
        std::this_thread::sleep_for(std::chrono::milliseconds(duration));
        SendKeyViaPostMessage(vkCode, false);
    } else {
        INPUT input = {0};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = vkCode;
        
        SendInput(1, &input, sizeof(INPUT));
        std::this_thread::sleep_for(std::chrono::milliseconds(duration));
        
        input.ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(1, &input, sizeof(INPUT));
    }
}

// Type string with human typing speed and patterns
void BDOAntiDetection::TypeString(const std::string& text, int wpm) {
    // Calculate base delay between characters
    int baseDelay = 60000 / (wpm * 5);  // Average 5 characters per word
    
    std::uniform_real_distribution<double> delayVariation(0.7, 1.3);
    std::uniform_real_distribution<double> mistakeChance(0.0, 1.0);
    
    for (size_t i = 0; i < text.length(); i++) {
        char c = text[i];
        
        // Occasionally make typos (and correct them)
        if (skillLevel < 0.8 && mistakeChance(rng) < (1.0 - skillLevel) * 0.05) {
            // Make a typo
            KeyPress(VkKeyScan(c + 1), false);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            // Realize mistake and backspace
            KeyPress(VK_BACK, false);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        
        // Type the correct character
        SHORT vk = VkKeyScan(c);
        if (vk != -1) {
            WORD vkCode = LOBYTE(vk);
            bool needShift = HIBYTE(vk) & 1;
            
            if (needShift) {
                INPUT shift = {0};
                shift.type = INPUT_KEYBOARD;
                shift.ki.wVk = VK_SHIFT;
                SendInput(1, &shift, sizeof(INPUT));
            }
            
            KeyPress(vkCode, false);
            
            if (needShift) {
                INPUT shift = {0};
                shift.type = INPUT_KEYBOARD;
                shift.ki.wVk = VK_SHIFT;
                shift.ki.dwFlags = KEYEVENTF_KEYUP;
                SendInput(1, &shift, sizeof(INPUT));
            }
        }
        
        // Variable delay between characters
        int delay = static_cast<int>(baseDelay * delayVariation(rng));
        
        // Longer pause after punctuation
        if (c == '.' || c == '!' || c == '?') {
            delay *= 2;
        } else if (c == ',' || c == ';') {
            delay = static_cast<int>(delay * 1.5);
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    }
}

// Execute combat rotation with human-like patterns
void BDOAntiDetection::CombatRotation(const std::vector<WORD>& skills) {
    std::uniform_real_distribution<double> mistakeRate(0.0, 1.0);
    std::uniform_int_distribution<int> reactionVariation(-50, 100);
    
    for (size_t i = 0; i < skills.size(); i++) {
        // Check if we should make a mistake
        if (mistakeRate(rng) < (1.0 - skillLevel) * 0.1) {
            AddCombatMistake();
            continue;
        }
        
        // Add reaction time between skills
        int reactionTime = BDO::MIN_HUMAN_REACTION + reactionVariation(rng);
        std::this_thread::sleep_for(std::chrono::milliseconds(reactionTime));
        
        // Execute skill
        KeyPress(skills[i], true);
        
        // Sometimes accidentally double-tap (common human mistake)
        if (mistakeRate(rng) < 0.02) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            KeyPress(skills[i], false);
        }
        
        // Record action
        RecordAction(BDO::ActionType::COMBAT_SKILL, reactionTime);
        
        // Check if pattern is too repetitive
        if (i % 5 == 0 && IsPatternTooRepetitive()) {
            IntroduceVariation();
        }
    }
}

// Add combat mistakes that humans make
void BDOAntiDetection::AddCombatMistake() {
    std::uniform_int_distribution<int> mistakeType(0, 3);
    
    switch (mistakeType(rng)) {
        case 0:
            // Wrong key press
            KeyPress(VK_NUMPAD1 + mistakeType(rng), false);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            break;
        case 1:
            // Delayed reaction (hesitation)
            std::this_thread::sleep_for(std::chrono::milliseconds(300 + pauseDist(rng) * 5));
            break;
        case 2:
            // Double press same key
            KeyPress(VK_SPACE, false);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            KeyPress(VK_SPACE, false);
            break;
        case 3:
            // Miss-click (click wrong area)
            AddMouseNoise(50);
            break;
    }
}

// Gathering pattern simulation
void BDOAntiDetection::GatheringPattern(int iterations) {
    std::uniform_int_distribution<int> gatherDelay(2800, 3200);  // Gathering takes ~3 seconds
    std::uniform_real_distribution<double> successRate(0.0, 1.0);
    
    for (int i = 0; i < iterations; i++) {
        // Look for gathering node (move mouse around)
        std::uniform_int_distribution<int> searchPattern(-100, 100);
        POINT pos;
        GetCursorPos(&pos);
        
        MoveMouseHuman(pos.x + searchPattern(rng), pos.y + searchPattern(rng));
        
        // Check if we should take a break
        if (i % 20 == 0 && ShouldTakeBreak()) {
            std::cout << "Taking a short break from gathering..." << std::endl;
            TakeBreak(1, 3);
        }
        
        // Attempt to gather (R key by default in BDO)
        KeyPress('R', true);
        
        // Wait for gathering animation
        int delay = gatherDelay(rng);
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
        
        // Sometimes fail gathering (energy depleted, tool broke, etc.)
        if (successRate(rng) > 0.95) {
            std::cout << "Gathering failed - checking tool/energy" << std::endl;
            KeyPress('I', true);  // Open inventory
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            KeyPress(VK_ESCAPE, true);  // Close
        }
        
        RecordAction(BDO::ActionType::GATHERING, delay);
        
        // Small movement between gathers
        AddMouseNoise(20);
        
        // Update fatigue
        UpdateFatigue();
    }
}

// Processing pattern (cooking, alchemy, etc.)
void BDOAntiDetection::ProcessingPattern(int iterations) {
    std::uniform_int_distribution<int> processDelay(800, 1200);
    
    for (int i = 0; i < iterations; i++) {
        // Start processing (L key)
        KeyPress('L', true);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        // Select recipe/material (simplified)
        KeyPress(VK_SPACE, true);
        
        // Processing time varies by type
        int delay = processDelay(rng) * 10;  // 8-12 seconds per batch
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
        
        // Collect result
        KeyPress('R', true);
        
        RecordAction(BDO::ActionType::PROCESSING, delay);
        
        // Check inventory occasionally
        if (i % 10 == 0) {
            KeyPress('I', true);
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
            KeyPress(VK_ESCAPE, true);
        }
    }
}

// Fishing pattern (AFK fishing simulation)
void BDOAntiDetection::FishingPattern() {
    std::uniform_int_distribution<int> fishingDelay(180000, 300000);  // 3-5 minutes per fish
    std::uniform_int_distribution<int> miniGameTiming(500, 1500);
    
    // Cast rod
    KeyPress(VK_SPACE, true);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    // Wait for bite
    int waitTime = fishingDelay(rng);
    std::cout << "Waiting for fish... (~" << (waitTime/1000) << " seconds)" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(waitTime));
    
    // Mini-game (simplified - just timing)
    KeyPress(VK_SPACE, true);
    std::this_thread::sleep_for(std::chrono::milliseconds(miniGameTiming(rng)));
    KeyPress(VK_SPACE, true);
    
    RecordAction(BDO::ActionType::FISHING, waitTime);
}

// Analyze current action pattern
void BDOAntiDetection::AnalyzeActionPattern() {
    if (actionHistory.size() < 10) return;
    
    // Count action types
    std::map<BDO::ActionType, int> actionCounts;
    std::vector<int> timingDeltas;
    
    for (size_t i = 0; i < actionHistory.size(); i++) {
        actionCounts[actionHistory[i].type]++;
        
        if (i > 0) {
            auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(
                actionHistory[i].timestamp - actionHistory[i-1].timestamp).count();
            timingDeltas.push_back(static_cast<int>(delta));
        }
    }
    
    // Calculate timing variance
    if (timingDeltas.size() > 2) {
        double mean = 0;
        for (int delta : timingDeltas) {
            mean += delta;
        }
        mean /= timingDeltas.size();
        
        double variance = 0;
        for (int delta : timingDeltas) {
            variance += pow(delta - mean, 2);
        }
        variance /= timingDeltas.size();
        
        // Low variance indicates bot-like behavior
        if (variance < 1000) {  // Less than 1 second variance
            std::cout << "Warning: Pattern too consistent, introducing variation" << std::endl;
            IntroduceVariation();
        }
    }
}

// Check if current pattern is too repetitive
bool BDOAntiDetection::IsPatternTooRepetitive() {
    if (actionHistory.size() < 20) return false;
    
    // Check last 20 actions for patterns
    std::vector<BDO::ActionType> recent;
    for (size_t i = actionHistory.size() - 20; i < actionHistory.size(); i++) {
        recent.push_back(actionHistory[i].type);
    }
    
    // Look for repeating sequences
    for (size_t seqLen = 2; seqLen <= 10; seqLen++) {
        bool isRepeating = true;
        for (size_t i = seqLen; i < recent.size(); i++) {
            if (recent[i] != recent[i % seqLen]) {
                isRepeating = false;
                break;
            }
        }
        if (isRepeating) {
            return true;
        }
    }
    
    return false;
}

// Introduce variation to break patterns
void BDOAntiDetection::IntroduceVariation() {
    std::uniform_int_distribution<int> variationType(0, 4);
    
    switch (variationType(rng)) {
        case 0:
            // Random pause
            std::cout << "Introducing random pause..." << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(500 + pauseDist(rng) * 20));
            break;
        case 1:
            // Check inventory/stats
            KeyPress('I', true);
            std::this_thread::sleep_for(std::chrono::milliseconds(2000 + pauseDist(rng) * 10));
            KeyPress(VK_ESCAPE, true);
            break;
        case 2:
            // Random camera movement
            AddMouseNoise(100);
            break;
        case 3:
            // Open and close map
            KeyPress('M', true);
            std::this_thread::sleep_for(std::chrono::milliseconds(1500));
            KeyPress(VK_ESCAPE, true);
            break;
        case 4:
            // Jump or dodge randomly
            KeyPress(VK_SPACE, true);
            break;
    }
}

// Take a break (AFK simulation)
void BDOAntiDetection::TakeBreak(int minMinutes, int maxMinutes) {
    std::uniform_int_distribution<int> breakDuration(minMinutes * 60000, maxMinutes * 60000);
    int breakTime = breakDuration(rng);
    
    std::cout << "Taking break for " << (breakTime / 60000) << " minutes..." << std::endl;
    
    // Simulate AFK behavior
    SimulateAFK();
    
    // Actually wait
    std::this_thread::sleep_for(std::chrono::milliseconds(breakTime));
    
    // Reset some stats after break
    fatigueLevel = std::max(0.0, fatigueLevel - 0.3);
    consecutiveActions = 0;
}

// Check if should take a break
bool BDOAntiDetection::ShouldTakeBreak() {
    double sessionHours = GetSessionDuration();
    
    // Mandatory break after certain time
    if (sessionHours > 2.0 && consecutiveActions > 100) {
        return true;
    }
    
    // Random chance based on fatigue
    std::uniform_real_distribution<double> breakChance(0.0, 1.0);
    return breakChance(rng) < (fatigueLevel * 0.1);  // Up to 10% chance when fully fatigued
}

// Get current session duration in hours
double BDOAntiDetection::GetSessionDuration() {
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::hours>(now - sessionStart);
    return duration.count();
}

// Simulate AFK behavior
void BDOAntiDetection::SimulateAFK() {
    std::cout << "Simulating AFK behavior..." << std::endl;
    
    // Move to safe spot (simplified)
    KeyPress('V', true);  // Auto-path to safe zone
    
    // Open character window and leave it open
    KeyPress('P', true);
    
    // Occasionally small mouse movements (like checking back)
    std::uniform_int_distribution<int> afkCheck(30000, 120000);  // 30s to 2min
    for (int i = 0; i < 3; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(afkCheck(rng)));
        AddMouseNoise(5);
    }
}

// Random window focus changes
void BDOAntiDetection::RandomWindowFocus() {
    std::uniform_real_distribution<double> focusChance(0.0, 1.0);
    
    if (focusChance(rng) < 0.05) {  // 5% chance
        // Alt-tab simulation
        INPUT alt = {0};
        alt.type = INPUT_KEYBOARD;
        alt.ki.wVk = VK_MENU;
        SendInput(1, &alt, sizeof(INPUT));
        
        INPUT tab = {0};
        tab.type = INPUT_KEYBOARD;
        tab.ki.wVk = VK_TAB;
        SendInput(1, &tab, sizeof(INPUT));
        
        tab.ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(1, &tab, sizeof(INPUT));
        
        alt.ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(1, &alt, sizeof(INPUT));
        
        // Wait a bit
        std::this_thread::sleep_for(std::chrono::milliseconds(2000 + pauseDist(rng) * 50));
        
        // Return to BDO
        if (bdoWindow) {
            SetForegroundWindow(bdoWindow);
        }
    }
}

// Check for GM presence
void BDOAntiDetection::CheckForGM() {
    // This would normally check chat/nearby players
    // Simplified version
    if (IsGMNearby()) {
        std::cout << "GM detected nearby! Stopping all actions." << std::endl;
        
        // Stop all automation
        SimulateAFK();
        
        // Take extended break
        TakeBreak(30, 60);
    }
}

// Configuration methods
void BDOAntiDetection::SetSkillLevel(double level) {
    skillLevel = std::max(0.0, std::min(1.0, level));
}

void BDOAntiDetection::SetFatigueRate(double rate) {
    // Adjust how quickly fatigue accumulates
    fatigueLevel *= rate;
}

void BDOAntiDetection::EnableAdvancedMode(bool enable) {
    if (enable) {
        currentMethod = InputMethod::POST_MESSAGE;
    } else {
        currentMethod = InputMethod::SEND_INPUT;
    }
}

// Helper: Calculate Bezier curve point
POINT BDOAntiDetection::GetBezierPoint(POINT start, POINT control1, POINT control2, POINT end, double t) {
    double u = 1 - t;
    double tt = t * t;
    double uu = u * u;
    double uuu = uu * u;
    double ttt = tt * t;
    
    POINT result;
    result.x = static_cast<LONG>(uuu * start.x + 3 * uu * t * control1.x + 3 * u * tt * control2.x + ttt * end.x);
    result.y = static_cast<LONG>(uuu * start.y + 3 * uu * t * control1.y + 3 * u * tt * control2.y + ttt * end.y);
    
    return result;
}

// Record action for pattern analysis
void BDOAntiDetection::RecordAction(BDO::ActionType type, int duration) {
    ActionRecord record;
    record.type = type;
    record.timestamp = std::chrono::steady_clock::now();
    record.duration = duration;
    
    actionHistory.push_back(record);
    
    // Keep history size manageable
    if (actionHistory.size() > MAX_HISTORY_SIZE) {
        actionHistory.pop_front();
    }
    
    consecutiveActions++;
}

// Calculate appropriate reaction time for action
int BDOAntiDetection::CalculateReactionTime(BDO::ActionType action) {
    int baseTime = BDO::MIN_HUMAN_REACTION;
    
    switch (action) {
        case BDO::ActionType::COMBAT_SKILL:
            baseTime = 150;  // Faster for combat
            break;
        case BDO::ActionType::MOVEMENT:
            baseTime = 100;  // Quick for movement
            break;
        case BDO::ActionType::GATHERING:
            baseTime = 300;  // Slower for gathering
            break;
        case BDO::ActionType::FISHING:
            baseTime = 500;  // Very relaxed for fishing
            break;
        case BDO::ActionType::PROCESSING:
            baseTime = 250;
            break;
        case BDO::ActionType::MENU_NAVIGATION:
            baseTime = 200;
            break;
        case BDO::ActionType::INVENTORY_MANAGEMENT:
            baseTime = 350;
            break;
    }
    
    // Add normal distribution variation
    int variation = static_cast<int>(reactionTimeDist(rng) - 250 + baseTime);
    return std::max(50, variation);  // Minimum 50ms
}

// Update fatigue based on actions
void BDOAntiDetection::UpdateFatigue() {
    // Fatigue increases with consecutive actions
    fatigueLevel += 0.001 * consecutiveActions;
    
    // But also naturally over time
    SimulateFatigue();
    
    // Cap at 1.0
    fatigueLevel = std::min(1.0, fatigueLevel);
}

// Check if GM is nearby (simplified)
bool BDOAntiDetection::IsGMNearby() {
    // This would normally check game memory or chat for GM indicators
    // For now, random very low chance
    std::uniform_real_distribution<double> gmChance(0.0, 1.0);
    return gmChance(rng) < 0.001;  // 0.1% chance
}

// Send key via PostMessage (less detectable)
void BDOAntiDetection::SendKeyViaPostMessage(WORD vkCode, bool keyDown) {
    if (!bdoWindow) return;
    
    LPARAM lParam = 1;
    lParam |= (0 << 16);  // Repeat count
    
    if (!keyDown) {
        lParam |= (1 << 30);  // Previous key state
        lParam |= (1 << 31);  // Transition state
    }
    
    PostMessage(bdoWindow, keyDown ? WM_KEYDOWN : WM_KEYUP, vkCode, lParam);
}

// Send mouse via PostMessage
void BDOAntiDetection::SendMouseViaPostMessage(int x, int y, bool leftClick) {
    if (!bdoWindow) return;
    
    LPARAM lParam = MAKELPARAM(x, y);
    
    if (leftClick) {
        PostMessage(bdoWindow, WM_LBUTTONDOWN, MK_LBUTTON, lParam);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        PostMessage(bdoWindow, WM_LBUTTONUP, 0, lParam);
    } else {
        PostMessage(bdoWindow, WM_MOUSEMOVE, 0, lParam);
    }
}

// ==== BDOMemory Class Implementation ====

BDOMemory::BDOMemory() : processHandle(nullptr), processId(0), baseAddress(0) {}

BDOMemory::~BDOMemory() {
    Detach();
}

bool BDOMemory::Attach() {
    // Find BDO process
    HWND hwnd = FindWindowA(BDO::WINDOW_CLASS, nullptr);
    if (!hwnd) {
        hwnd = FindWindowA(nullptr, BDO::WINDOW_TITLE);
    }
    
    if (!hwnd) {
        std::cout << "BDO window not found for memory reading" << std::endl;
        return false;
    }
    
    GetWindowThreadProcessId(hwnd, &processId);
    if (!processId) {
        std::cout << "Failed to get BDO process ID" << std::endl;
        return false;
    }
    
    // Open process with read permissions
    processHandle = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, processId);
    if (!processHandle) {
        std::cout << "Failed to open BDO process (need admin rights)" << std::endl;
        return false;
    }
    
    // Get base address (simplified - would need to enumerate modules)
    baseAddress = 0x140000000;  // Default for 64-bit executables
    
    std::cout << "Attached to BDO process: " << processId << std::endl;
    return true;
}

void BDOMemory::Detach() {
    if (processHandle) {
        CloseHandle(processHandle);
        processHandle = nullptr;
    }
    processId = 0;
    baseAddress = 0;
}

template<typename T>
bool BDOMemory::ReadMemory(uintptr_t address, T& value) {
    if (!processHandle) return false;
    
    SIZE_T bytesRead;
    if (ReadProcessMemory(processHandle, reinterpret_cast<LPCVOID>(address), &value, sizeof(T), &bytesRead)) {
        return bytesRead == sizeof(T);
    }
    return false;
}

bool BDOMemory::GetPlayerHealth(int& health, int& maxHealth) {
    if (!processHandle) return false;
    
    // These offsets would need to be updated for current BDO version
    uintptr_t playerBase = baseAddress + Offsets::PLAYER_BASE;
    uintptr_t healthAddr = playerBase + Offsets::HEALTH_OFFSET;
    
    // Read health values (simplified)
    ReadMemory(healthAddr, health);
    ReadMemory(healthAddr + 4, maxHealth);
    
    return true;
}

bool BDOMemory::GetPlayerPosition(float& x, float& y, float& z) {
    if (!processHandle) return false;
    
    uintptr_t playerBase = baseAddress + Offsets::PLAYER_BASE;
    
    ReadMemory(playerBase + Offsets::POS_X_OFFSET, x);
    ReadMemory(playerBase + Offsets::POS_Y_OFFSET, y);
    ReadMemory(playerBase + Offsets::POS_Z_OFFSET, z);
    
    return true;
}

bool BDOMemory::IsPlayerInCombat() {
    // Would check combat flag in memory
    // Simplified version
    return false;
}

bool BDOMemory::IsPlayerFishing() {
    // Would check fishing state in memory
    // Simplified version
    return false;
}

// Explicit template instantiations
template bool BDOMemory::ReadMemory<int>(uintptr_t address, int& value);
template bool BDOMemory::ReadMemory<float>(uintptr_t address, float& value);
template bool BDOMemory::ReadMemory<bool>(uintptr_t address, bool& value);