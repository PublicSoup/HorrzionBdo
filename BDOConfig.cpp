#include "BDOConfig.h"
#include <iostream>
#include <filesystem>
#include <sstream>

// Save configuration to JSON file
bool BDOConfig::SaveToFile(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for saving: " << filename << std::endl;
        return false;
    }
    
    // Create JSON structure (simplified without actual JSON library for compatibility)
    file << "{\n";
    file << "  \"profileName\": \"" << profileName << "\",\n";
    file << "  \"characterClass\": \"" << characterClass << "\",\n";
    file << "  \"characterLevel\": " << characterLevel << ",\n";
    
    // Behavior settings
    file << "  \"behavior\": {\n";
    file << "    \"skillLevel\": " << behavior.skillLevel << ",\n";
    file << "    \"fatigueRate\": " << behavior.fatigueRate << ",\n";
    file << "    \"minReactionTime\": " << behavior.minReactionTime << ",\n";
    file << "    \"maxReactionTime\": " << behavior.maxReactionTime << ",\n";
    file << "    \"mistakeChance\": " << behavior.mistakeChance << ",\n";
    file << "    \"useAdvancedInput\": " << (behavior.useAdvancedInput ? "true" : "false") << "\n";
    file << "  },\n";
    
    // Combat settings
    file << "  \"combat\": {\n";
    file << "    \"skillRotation\": [";
    for (size_t i = 0; i < combat.skillRotation.size(); i++) {
        file << "\"" << combat.skillRotation[i] << "\"";
        if (i < combat.skillRotation.size() - 1) file << ", ";
    }
    file << "],\n";
    file << "    \"combatRange\": " << combat.combatRange << ",\n";
    file << "    \"useEvasion\": " << (combat.useEvasion ? "true" : "false") << ",\n";
    file << "    \"usePotions\": " << (combat.usePotions ? "true" : "false") << ",\n";
    file << "    \"potionHealthThreshold\": " << combat.potionHealthThreshold << "\n";
    file << "  },\n";
    
    // Life skill settings
    file << "  \"lifeSkills\": {\n";
    file << "    \"primaryLifeSkill\": \"" << lifeSkills.primaryLifeSkill << "\",\n";
    file << "    \"energyThreshold\": " << lifeSkills.energyThreshold << ",\n";
    file << "    \"weightLimit\": " << lifeSkills.weightLimit << ",\n";
    file << "    \"autoRepairTools\": " << (lifeSkills.autoRepairTools ? "true" : "false") << ",\n";
    file << "    \"autoSellJunk\": " << (lifeSkills.autoSellJunk ? "true" : "false") << ",\n";
    file << "    \"processingBatchSize\": " << lifeSkills.processingBatchSize << "\n";
    file << "  },\n";
    
    // Movement settings
    file << "  \"movement\": {\n";
    file << "    \"useAutoPath\": " << (movement.useAutoPath ? "true" : "false") << ",\n";
    file << "    \"mouseSpeed\": " << movement.mouseSpeed << ",\n";
    file << "    \"mouseCurveIntensity\": " << movement.mouseCurveIntensity << ",\n";
    file << "    \"addMouseNoise\": " << (movement.addMouseNoise ? "true" : "false") << ",\n";
    file << "    \"noiseIntensity\": " << movement.noiseIntensity << "\n";
    file << "  },\n";
    
    // Session settings
    file << "  \"session\": {\n";
    file << "    \"maxSessionHours\": " << session.maxSessionHours << ",\n";
    file << "    \"breakIntervalMinutes\": " << session.breakIntervalMinutes << ",\n";
    file << "    \"breakDurationMin\": " << session.breakDurationMin << ",\n";
    file << "    \"breakDurationMax\": " << session.breakDurationMax << ",\n";
    file << "    \"simulateAFK\": " << (session.simulateAFK ? "true" : "false") << ",\n";
    file << "    \"randomSchedule\": " << (session.randomSchedule ? "true" : "false") << "\n";
    file << "  },\n";
    
    // Anti-detection settings
    file << "  \"antiDetection\": {\n";
    file << "    \"enableGMDetection\": " << (antiDetection.enableGMDetection ? "true" : "false") << ",\n";
    file << "    \"enablePatternAnalysis\": " << (antiDetection.enablePatternAnalysis ? "true" : "false") << ",\n";
    file << "    \"patternCheckInterval\": " << antiDetection.patternCheckInterval << ",\n";
    file << "    \"randomWindowSwitch\": " << (antiDetection.randomWindowSwitch ? "true" : "false") << ",\n";
    file << "    \"humanizeTyping\": " << (antiDetection.humanizeTyping ? "true" : "false") << ",\n";
    file << "    \"typingWPM\": " << antiDetection.typingWPM << "\n";
    file << "  }\n";
    file << "}\n";
    
    file.close();
    std::cout << "Configuration saved to " << filename << std::endl;
    return true;
}

// Load configuration from JSON file
bool BDOConfig::LoadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for loading: " << filename << std::endl;
        return false;
    }
    
    // Simple JSON parsing (would use proper JSON library in production)
    std::string line;
    while (std::getline(file, line)) {
        // Parse each line (simplified - real implementation would use JSON library)
        if (line.find("\"profileName\"") != std::string::npos) {
            size_t start = line.find(":") + 3;
            size_t end = line.rfind("\"");
            if (start < end) {
                profileName = line.substr(start, end - start);
            }
        }
        else if (line.find("\"characterClass\"") != std::string::npos) {
            size_t start = line.find(":") + 3;
            size_t end = line.rfind("\"");
            if (start < end) {
                characterClass = line.substr(start, end - start);
            }
        }
        else if (line.find("\"characterLevel\"") != std::string::npos) {
            size_t start = line.find(":") + 1;
            characterLevel = std::stoi(line.substr(start));
        }
        // ... continue parsing other fields
    }
    
    file.close();
    std::cout << "Configuration loaded from " << filename << std::endl;
    return true;
}

// Get default profile configuration
BDOConfig BDOConfig::GetDefaultProfile() {
    BDOConfig config;
    
    config.profileName = "Default";
    config.characterClass = "Warrior";
    config.characterLevel = 60;
    
    // Behavior
    config.behavior.skillLevel = 0.5;
    config.behavior.fatigueRate = 1.0;
    config.behavior.minReactionTime = 150;
    config.behavior.maxReactionTime = 350;
    config.behavior.mistakeChance = 0.05;
    config.behavior.useAdvancedInput = true;
    
    // Combat
    config.combat.skillRotation = {"Q", "LMB", "RMB", "F"};
    config.combat.combatRange = 200;
    config.combat.useEvasion = true;
    config.combat.usePotions = true;
    config.combat.potionHealthThreshold = 50;
    
    // Life skills
    config.lifeSkills.primaryLifeSkill = "Gathering";
    config.lifeSkills.energyThreshold = 50;
    config.lifeSkills.weightLimit = 90;
    config.lifeSkills.autoRepairTools = true;
    config.lifeSkills.autoSellJunk = false;
    config.lifeSkills.processingBatchSize = 100;
    
    // Movement
    config.movement.useAutoPath = false;
    config.movement.mouseSpeed = 1.0;
    config.movement.mouseCurveIntensity = 0.3;
    config.movement.addMouseNoise = true;
    config.movement.noiseIntensity = 3;
    
    // Session
    config.session.maxSessionHours = 4;
    config.session.breakIntervalMinutes = 120;
    config.session.breakDurationMin = 10;
    config.session.breakDurationMax = 30;
    config.session.simulateAFK = true;
    config.session.randomSchedule = true;
    
    // Anti-detection
    config.antiDetection.enableGMDetection = true;
    config.antiDetection.enablePatternAnalysis = true;
    config.antiDetection.patternCheckInterval = 50;
    config.antiDetection.randomWindowSwitch = true;
    config.antiDetection.humanizeTyping = true;
    config.antiDetection.typingWPM = 60;
    
    return config;
}

// Get beginner-friendly profile (more mistakes, slower actions)
BDOConfig BDOConfig::GetBeginnerProfile() {
    BDOConfig config = GetDefaultProfile();
    
    config.profileName = "Beginner";
    config.behavior.skillLevel = 0.2;
    config.behavior.minReactionTime = 250;
    config.behavior.maxReactionTime = 500;
    config.behavior.mistakeChance = 0.15;
    config.movement.mouseSpeed = 0.7;
    config.antiDetection.typingWPM = 40;
    
    return config;
}

// Get intermediate profile
BDOConfig BDOConfig::GetIntermediateProfile() {
    BDOConfig config = GetDefaultProfile();
    
    config.profileName = "Intermediate";
    config.behavior.skillLevel = 0.6;
    config.behavior.minReactionTime = 120;
    config.behavior.maxReactionTime = 300;
    config.behavior.mistakeChance = 0.03;
    config.movement.mouseSpeed = 1.2;
    config.antiDetection.typingWPM = 70;
    
    return config;
}

// Get advanced profile (expert player simulation)
BDOConfig BDOConfig::GetAdvancedProfile() {
    BDOConfig config = GetDefaultProfile();
    
    config.profileName = "Advanced";
    config.behavior.skillLevel = 0.9;
    config.behavior.minReactionTime = 100;
    config.behavior.maxReactionTime = 250;
    config.behavior.mistakeChance = 0.01;
    config.movement.mouseSpeed = 1.5;
    config.movement.mouseCurveIntensity = 0.1;  // More direct movements
    config.antiDetection.typingWPM = 90;
    
    // Advanced combat rotation
    config.combat.skillRotation = {"C", "Q", "S+LMB", "W+RMB", "SHIFT+Q", "E", "F", "SHIFT+F"};
    
    return config;
}

// Get gathering-focused profile
BDOConfig BDOConfig::GetGatheringProfile() {
    BDOConfig config = GetDefaultProfile();
    
    config.profileName = "Gatherer";
    config.lifeSkills.primaryLifeSkill = "Gathering";
    config.session.maxSessionHours = 6;  // Longer sessions for gathering
    config.behavior.skillLevel = 0.4;  // Doesn't need high skill for gathering
    config.behavior.mistakeChance = 0.02;  // Few mistakes
    
    return config;
}

// Get fishing-focused profile
BDOConfig BDOConfig::GetFishingProfile() {
    BDOConfig config = GetDefaultProfile();
    
    config.profileName = "Fisher";
    config.lifeSkills.primaryLifeSkill = "Fishing";
    config.session.maxSessionHours = 8;  // Can fish for very long
    config.session.breakIntervalMinutes = 180;  // Less frequent breaks
    config.behavior.skillLevel = 0.3;  // Low skill needed
    config.behavior.minReactionTime = 500;  // Slower reactions OK for fishing
    config.behavior.maxReactionTime = 1500;
    
    return config;
}

// Get grinding/combat-focused profile
BDOConfig BDOConfig::GetGrindingProfile() {
    BDOConfig config = GetDefaultProfile();
    
    config.profileName = "Grinder";
    config.combat.useEvasion = true;
    config.combat.usePotions = true;
    config.combat.potionHealthThreshold = 60;  // Use potions earlier
    config.behavior.skillLevel = 0.7;  // Need good skill for combat
    config.behavior.minReactionTime = 100;
    config.behavior.maxReactionTime = 200;  // Fast reactions for combat
    config.session.breakIntervalMinutes = 90;  // More frequent breaks due to intensity
    
    return config;
}

// ==== Profile Manager Implementation ====

BDOProfileManager::BDOProfileManager(const std::string& dir) : profileDirectory(dir) {
    // Create profile directory if it doesn't exist
    if (!std::filesystem::exists(profileDirectory)) {
        std::filesystem::create_directories(profileDirectory);
    }
    currentProfile = "Default";
}

// Load a profile by name
bool BDOProfileManager::LoadProfile(const std::string& name) {
    std::string filepath = profileDirectory + name + ".json";
    
    if (!std::filesystem::exists(filepath)) {
        std::cerr << "Profile not found: " << name << std::endl;
        return false;
    }
    
    BDOConfig config;
    if (config.LoadFromFile(filepath)) {
        profiles[name] = config;
        return true;
    }
    
    return false;
}

// Save a profile
bool BDOProfileManager::SaveProfile(const std::string& name, const BDOConfig& config) {
    std::string filepath = profileDirectory + name + ".json";
    
    BDOConfig configCopy = config;
    configCopy.profileName = name;
    
    if (configCopy.SaveToFile(filepath)) {
        profiles[name] = configCopy;
        return true;
    }
    
    return false;
}

// Delete a profile
bool BDOProfileManager::DeleteProfile(const std::string& name) {
    if (name == "Default") {
        std::cerr << "Cannot delete default profile" << std::endl;
        return false;
    }
    
    std::string filepath = profileDirectory + name + ".json";
    
    if (std::filesystem::exists(filepath)) {
        std::filesystem::remove(filepath);
        profiles.erase(name);
        
        if (currentProfile == name) {
            currentProfile = "Default";
        }
        
        return true;
    }
    
    return false;
}

// List all available profiles
std::vector<std::string> BDOProfileManager::ListProfiles() {
    std::vector<std::string> profileList;
    
    for (const auto& entry : std::filesystem::directory_iterator(profileDirectory)) {
        if (entry.path().extension() == ".json") {
            profileList.push_back(entry.path().stem().string());
        }
    }
    
    return profileList;
}

// Get current profile
BDOConfig& BDOProfileManager::GetCurrentProfile() {
    if (profiles.find(currentProfile) == profiles.end()) {
        // Load default if current doesn't exist
        profiles[currentProfile] = BDOConfig::GetDefaultProfile();
    }
    
    return profiles[currentProfile];
}

// Set current profile
bool BDOProfileManager::SetCurrentProfile(const std::string& name) {
    if (profiles.find(name) == profiles.end()) {
        if (!LoadProfile(name)) {
            return false;
        }
    }
    
    currentProfile = name;
    return true;
}

// Reload current profile from disk
bool BDOProfileManager::ReloadCurrentProfile() {
    return LoadProfile(currentProfile);
}

// Export profile to specific path
bool BDOProfileManager::ExportProfile(const std::string& name, const std::string& path) {
    if (profiles.find(name) == profiles.end()) {
        if (!LoadProfile(name)) {
            return false;
        }
    }
    
    return profiles[name].SaveToFile(path);
}

// Import profile from path
bool BDOProfileManager::ImportProfile(const std::string& path) {
    BDOConfig config;
    if (!config.LoadFromFile(path)) {
        return false;
    }
    
    // Extract name from config or filename
    std::filesystem::path p(path);
    std::string name = p.stem().string();
    
    return SaveProfile(name, config);
}

// Create default profiles
void BDOProfileManager::CreateDefaultProfiles() {
    SaveProfile("Default", BDOConfig::GetDefaultProfile());
    SaveProfile("Beginner", BDOConfig::GetBeginnerProfile());
    SaveProfile("Intermediate", BDOConfig::GetIntermediateProfile());
    SaveProfile("Advanced", BDOConfig::GetAdvancedProfile());
    SaveProfile("Gatherer", BDOConfig::GetGatheringProfile());
    SaveProfile("Fisher", BDOConfig::GetFishingProfile());
    SaveProfile("Grinder", BDOConfig::GetGrindingProfile());
    
    std::cout << "Default profiles created successfully" << std::endl;
}