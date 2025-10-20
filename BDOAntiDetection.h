#ifndef BDO_ANTIDETECTION_H
#define BDO_ANTIDETECTION_H

#include <windows.h>
#include <random>
#include <chrono>
#include <thread>
#include <vector>
#include <deque>
#include <memory>

// BDO-specific constants
namespace BDO {
    // Common BDO window class names
    constexpr const char* WINDOW_CLASS = "BlackDesertWindowClass";
    constexpr const char* WINDOW_TITLE = "Black Desert";
    
    // Timing patterns specific to BDO
    constexpr int MIN_HUMAN_REACTION = 150;  // Minimum human reaction time in ms
    constexpr int MAX_HUMAN_REACTION = 350;  // Average human reaction time
    
    // BDO action types for pattern variation
    enum class ActionType {
        COMBAT_SKILL,
        MOVEMENT,
        GATHERING,
        FISHING,
        PROCESSING,
        MENU_NAVIGATION,
        INVENTORY_MANAGEMENT
    };
}

class BDOAntiDetection {
private:
    // Random number generation
    std::mt19937 rng;
    std::normal_distribution<double> reactionTimeDist;
    std::uniform_real_distribution<double> jitterDist;
    std::uniform_int_distribution<int> pauseDist;
    std::poisson_distribution<int> mistakeDist;
    
    // Pattern tracking for avoiding detection
    struct ActionRecord {
        BDO::ActionType type;
        std::chrono::steady_clock::time_point timestamp;
        int duration;
    };
    std::deque<ActionRecord> actionHistory;
    static constexpr size_t MAX_HISTORY_SIZE = 100;
    
    // Behavioral patterns
    double fatigueLevel;
    double skillLevel;
    int consecutiveActions;
    std::chrono::steady_clock::time_point sessionStart;
    
    // BDO-specific handles
    HWND bdoWindow;
    
    // Input simulation modes
    enum class InputMethod {
        SEND_INPUT,      // SendInput API (more detectable)
        POST_MESSAGE,    // PostMessage (less detectable for BDO)
        DIRECT_INPUT     // DirectInput simulation
    };
    InputMethod currentMethod;
    
public:
    BDOAntiDetection();
    ~BDOAntiDetection();
    
    // Initialize with BDO window
    bool Initialize();
    bool FindBDOWindow();
    
    // Enhanced timing functions for BDO
    void AddHumanDelay(BDO::ActionType action);
    void SimulateFatigue();
    void AddMicroPause();
    
    // Mouse movement with curves (BDO tracks linear movements)
    void MoveMouseBezier(int targetX, int targetY, int duration = 0);
    void MoveMouseHuman(int targetX, int targetY);
    void AddMouseNoise(int intensity = 3);
    
    // Keyboard input with human characteristics
    void KeyPress(WORD vkCode, bool addVariation = true);
    void KeyHold(WORD vkCode, int duration);
    void TypeString(const std::string& text, int wpm = 60);
    
    // Combat-specific anti-detection
    void CombatRotation(const std::vector<WORD>& skills);
    void AddCombatMistake();  // Occasionally miss skills like humans
    
    // Gathering/Life skill specific
    void GatheringPattern(int iterations);
    void ProcessingPattern(int iterations);
    void FishingPattern();
    
    // Pattern analysis and adjustment
    void AnalyzeActionPattern();
    bool IsPatternTooRepetitive();
    void IntroduceVariation();
    
    // Session management
    void TakeBreak(int minMinutes = 5, int maxMinutes = 15);
    bool ShouldTakeBreak();
    double GetSessionDuration();  // Returns hours
    
    // Advanced anti-detection features
    void SimulateAFK();
    void RandomWindowFocus();
    void CheckForGM();  // Look for GM whispers or nearby GMs
    
    // Configuration
    void SetSkillLevel(double level);  // 0.0 (beginner) to 1.0 (expert)
    void SetFatigueRate(double rate);
    void EnableAdvancedMode(bool enable);
    
private:
    // Helper functions
    POINT GetBezierPoint(POINT start, POINT control1, POINT control2, POINT end, double t);
    void RecordAction(BDO::ActionType type, int duration);
    int CalculateReactionTime(BDO::ActionType action);
    void UpdateFatigue();
    bool IsGMNearby();
    
    // Input method helpers
    void SendKeyViaPostMessage(WORD vkCode, bool keyDown);
    void SendMouseViaPostMessage(int x, int y, bool leftClick = false);
};

// Utility class for BDO memory reading (requires elevation)
class BDOMemoryReader {
private:
    HANDLE processHandle;
    DWORD processId;
    uintptr_t baseAddress;
    
public:
    BDOMemoryReader();
    ~BDOMemoryReader();
    
    bool Attach();
    void Detach();
    
    // Safe memory reading with anti-detection
    template<typename T>
    bool ReadMemory(uintptr_t address, T& value);
    
    // Common BDO offsets (these change with updates)
    struct Offsets {
        static constexpr uintptr_t PLAYER_BASE = 0x3A5B2C8;  // Example, needs updating
        static constexpr uintptr_t HEALTH_OFFSET = 0x2C;
        static constexpr uintptr_t MANA_OFFSET = 0x30;
        static constexpr uintptr_t STAMINA_OFFSET = 0x34;
        static constexpr uintptr_t POS_X_OFFSET = 0x40;
        static constexpr uintptr_t POS_Y_OFFSET = 0x44;
        static constexpr uintptr_t POS_Z_OFFSET = 0x48;
    };
    
    // High-level functions
    bool GetPlayerHealth(int& health, int& maxHealth);
    bool GetPlayerPosition(float& x, float& y, float& z);
    bool IsPlayerInCombat();
    bool IsPlayerFishing();
};

#endif // BDO_ANTIDETECTION_H