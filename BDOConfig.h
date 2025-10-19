#ifndef BDO_CONFIG_H
#define BDO_CONFIG_H

#include <string>
#include <map>
#include <vector>
#include <fstream>
// JSON parsing implemented manually for compatibility

// Configuration structure for BDO automation
struct BDOConfig {
    // Profile information
    std::string profileName;
    std::string characterClass;  // Warrior, Sorceress, Berserker, etc.
    int characterLevel;
    
    // Behavior settings
    struct BehaviorSettings {
        double skillLevel;        // 0.0-1.0 (noob to expert)
        double fatigueRate;       // How fast fatigue accumulates
        int minReactionTime;      // Minimum human reaction time (ms)
        int maxReactionTime;      // Maximum human reaction time (ms)
        double mistakeChance;     // Chance of making mistakes (0.0-1.0)
        bool useAdvancedInput;    // Use PostMessage instead of SendInput
    } behavior;
    
    // Combat settings
    struct CombatSettings {
        std::vector<std::string> skillRotation;  // Skill key sequence
        std::map<std::string, int> skillCooldowns;  // Skill name -> cooldown in ms
        int combatRange;          // Preferred combat distance
        bool useEvasion;          // Use dodge/block
        bool usePotions;          // Auto use potions
        int potionHealthThreshold; // HP % to use potion
        std::vector<std::string> buffSequence;  // Pre-combat buffs
    } combat;
    
    // Gathering/Life skills settings
    struct LifeSkillSettings {
        std::string primaryLifeSkill;  // Gathering, Fishing, Processing, etc.
        int energyThreshold;      // Minimum energy before break
        int weightLimit;          // Max weight % before returning
        bool autoRepairTools;     // Auto repair gathering tools
        bool autoSellJunk;        // Auto sell gray items
        std::vector<std::string> gatheringSpots;  // Coordinates or area names
        int processingBatchSize;  // Items per processing batch
    } lifeSkills;
    
    // Movement settings
    struct MovementSettings {
        bool useAutoPath;         // Use in-game auto-pathing
        double mouseSpeed;        // Mouse movement speed multiplier
        double mouseCurveIntensity; // How curved mouse movements are
        bool addMouseNoise;       // Add random noise to mouse
        int noiseIntensity;       // Pixel range for noise
    } movement;
    
    // Break/Session settings
    struct SessionSettings {
        int maxSessionHours;      // Max hours before mandatory break
        int breakIntervalMinutes; // Take break every X minutes
        int breakDurationMin;     // Minimum break duration
        int breakDurationMax;     // Maximum break duration
        bool simulateAFK;         // Simulate AFK during breaks
        bool randomSchedule;      // Randomize play schedule
        std::vector<std::pair<int, int>> activeHours;  // Hour ranges when to play
    } session;
    
    // Detection avoidance
    struct AntiDetectionSettings {
        bool enableGMDetection;   // Check for GM presence
        bool enablePatternAnalysis; // Analyze and vary patterns
        int patternCheckInterval; // Check pattern every X actions
        bool randomWindowSwitch;  // Randomly alt-tab
        bool humanizeTyping;      // Add typos and corrections
        int typingWPM;           // Words per minute when typing
    } antiDetection;
    
    // Hotkeys
    std::map<std::string, int> hotkeys;  // Action name -> VK code
    
    // Save/Load functions
    bool SaveToFile(const std::string& filename);
    bool LoadFromFile(const std::string& filename);
    
    // Preset profiles
    static BDOConfig GetDefaultProfile();
    static BDOConfig GetBeginnerProfile();
    static BDOConfig GetIntermediateProfile();
    static BDOConfig GetAdvancedProfile();
    static BDOConfig GetGatheringProfile();
    static BDOConfig GetFishingProfile();
    static BDOConfig GetGrindingProfile();
};

// Profile manager for multiple characters/accounts
class BDOProfileManager {
private:
    std::map<std::string, BDOConfig> profiles;
    std::string currentProfile;
    std::string profileDirectory;
    
public:
    BDOProfileManager(const std::string& dir = "./profiles/");
    
    // Profile management
    bool LoadProfile(const std::string& name);
    bool SaveProfile(const std::string& name, const BDOConfig& config);
    bool DeleteProfile(const std::string& name);
    std::vector<std::string> ListProfiles();
    
    // Current profile operations
    BDOConfig& GetCurrentProfile();
    bool SetCurrentProfile(const std::string& name);
    bool ReloadCurrentProfile();
    
    // Import/Export
    bool ExportProfile(const std::string& name, const std::string& path);
    bool ImportProfile(const std::string& path);
    
    // Profile templates
    void CreateDefaultProfiles();
};

// Class rotation configurations for different classes
namespace ClassRotations {
    // Warrior
    const std::vector<std::string> WARRIOR_PVE = {"Q", "LMB", "RMB", "S+LMB", "S+RMB", "SHIFT+Q"};
    const std::vector<std::string> WARRIOR_PVP = {"C", "Q", "S+LMB", "W+RMB", "SHIFT+Q", "E"};
    
    // Sorceress
    const std::vector<std::string> SORC_PVE = {"SHIFT+LMB", "SHIFT+RMB", "S+LMB", "S+RMB", "SHIFT+F"};
    const std::vector<std::string> SORC_PVP = {"W+F", "SHIFT+LMB", "C", "SHIFT+RMB", "S+F", "SHIFT+Q"};
    
    // Berserker
    const std::vector<std::string> BERSERK_PVE = {"LMB", "RMB", "F", "S+LMB", "W+F", "SHIFT+LMB"};
    const std::vector<std::string> BERSERK_PVP = {"Q", "E", "W+F", "GRAB", "S+LMB", "SHIFT+Q"};
    
    // Add more classes as needed...
}

// Skill cooldown database
namespace SkillCooldowns {
    const std::map<std::string, int> WARRIOR_COOLDOWNS = {
        {"SHIFT+Q", 7000},   // Grave Digging
        {"C", 5000},         // Shield Charge  
        {"E", 10000},        // Grab
        {"Q", 6000}          // Spinning Slash
    };
    
    const std::map<std::string, int> SORC_COOLDOWNS = {
        {"SHIFT+Q", 9000},   // Night Crow
        {"W+F", 7000},       // Violation
        {"C", 3000},         // Midnight Stinger
        {"SHIFT+F", 12000}   // Dream of Doom
    };
    
    // Add more classes...
}

#endif // BDO_CONFIG_H