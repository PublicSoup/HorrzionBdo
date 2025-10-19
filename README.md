# Black Desert Online Advanced Anti-Detection System

## 🎮 Overview

A sophisticated anti-detection system specifically designed for Black Desert Online automation. This system simulates human-like behavior patterns to evade bot detection mechanisms while providing a comprehensive framework for game automation.

## ⚡ Key Features

### Core Anti-Detection
- **Human-like Mouse Movement**: Bezier curves, overshoot correction, natural tremor
- **Realistic Timing Patterns**: Variable reaction times based on skill level and fatigue
- **Mistake Simulation**: Intentional errors that humans make (typos, miss-clicks, wrong keys)
- **Pattern Analysis**: Detects and breaks repetitive patterns automatically
- **Fatigue System**: Simulates tiredness over time affecting performance

### BDO-Specific Features
- **Combat Rotation System**: Class-specific skill rotations with realistic execution
- **Life Skills Automation**: Gathering, fishing, processing with human patterns
- **GM Detection**: Monitors for GM presence and takes evasive action
- **Memory Reading**: Safe game state reading with anti-detection measures
- **Profile Management**: Multiple character profiles with different skill levels

### Advanced Techniques
- **PostMessage Input**: Less detectable than SendInput for BDO
- **Session Management**: Automatic breaks and AFK simulation
- **Window Focus Variation**: Random alt-tabs like real players
- **Dynamic Pattern Variation**: Continuously evolves behavior to avoid detection

## 📁 Project Structure

```
/workspace/
├── AntiDetection.h           # Basic anti-detection interface
├── AntiDetection.cpp         # Basic implementation
├── BDOAntiDetection.h        # BDO-specific anti-detection
├── BDOAntiDetection.cpp      # BDO implementation with advanced features
├── BDOConfig.h               # Configuration and profile system
├── BDOConfig.cpp             # Profile management implementation
├── main.cpp                  # Interactive demo application
├── Makefile                  # Build configuration
├── README.md                 # This documentation
└── profiles/                 # Profile storage directory (auto-created)
    ├── Default.json
    ├── Beginner.json
    ├── Intermediate.json
    ├── Advanced.json
    ├── Gatherer.json
    ├── Fisher.json
    └── Grinder.json
```

## 🛠️ Building the Project

### Requirements
- Windows OS (Windows 10/11 recommended)
- C++17 compatible compiler (MinGW-w64, MSVC 2017+)
- Windows SDK for user32.lib

### Quick Build
```bash
# Standard build
make

# Debug build with symbols
make debug

# Optimized release build
make release

# Clean and rebuild
make rebuild
```

### Manual Compilation
```bash
g++ -std=c++17 -O2 main.cpp AntiDetection.cpp BDOAntiDetection.cpp BDOConfig.cpp -o bdo_antidetection -luser32 -lstdc++fs
```

## 🚀 Usage

### Basic Usage
```cpp
#include "BDOAntiDetection.h"

int main() {
    // Create anti-detection instance
    BDOAntiDetection bdoAntiDet;
    
    // Initialize (finds BDO window)
    if (bdoAntiDet.Initialize()) {
        // Set skill level (0.0 = noob, 1.0 = expert)
        bdoAntiDet.SetSkillLevel(0.6);
        
        // Execute combat rotation
        std::vector<WORD> skills = {'Q', 'E', 'R', 'F'};
        bdoAntiDet.CombatRotation(skills);
        
        // Gathering automation
        bdoAntiDet.GatheringPattern(10);  // 10 iterations
    }
    
    return 0;
}
```

### Profile System
```cpp
#include "BDOConfig.h"

// Create profile manager
BDOProfileManager profiles("./profiles/");

// Load a profile
profiles.SetCurrentProfile("Advanced");
auto& config = profiles.GetCurrentProfile();

// Create custom profile
BDOConfig custom = BDOConfig::GetDefaultProfile();
custom.behavior.skillLevel = 0.8;
custom.combat.skillRotation = {"Q", "W", "E", "R"};
profiles.SaveProfile("MyProfile", custom);
```

### Human-like Mouse Movement
```cpp
// Smooth Bezier curve movement
bdoAntiDet.MoveMouseBezier(targetX, targetY, 500);  // 500ms duration

// Movement with overshoot and correction
bdoAntiDet.MoveMouseHuman(targetX, targetY);

// Add natural mouse tremor
bdoAntiDet.AddMouseNoise(3);  // 3 pixel intensity
```

## 🎯 Class-Specific Rotations

### Warrior
- **PvE**: Q → LMB → RMB → S+LMB → S+RMB → Shift+Q
- **PvP**: C → Q → S+LMB → W+RMB → Shift+Q → E

### Sorceress  
- **PvE**: Shift+LMB → Shift+RMB → S+LMB → S+RMB → Shift+F
- **PvP**: W+F → Shift+LMB → C → Shift+RMB → S+F → Shift+Q

### Berserker
- **PvE**: LMB → RMB → F → S+LMB → W+F → Shift+LMB
- **PvP**: Q → E → W+F → Grab → S+LMB → Shift+Q

## ⚙️ Configuration Options

### Behavior Settings
- `skillLevel`: 0.0-1.0 (affects mistake rate and timing)
- `fatigueRate`: How quickly fatigue accumulates
- `minReactionTime`: Minimum human reaction time (ms)
- `maxReactionTime`: Maximum human reaction time (ms)
- `mistakeChance`: Probability of making mistakes

### Combat Settings
- `skillRotation`: Sequence of skills to execute
- `skillCooldowns`: Cooldown timers for each skill
- `combatRange`: Preferred distance from target
- `useEvasion`: Enable dodge/block mechanics
- `usePotions`: Auto-consume potions at threshold

### Session Management
- `maxSessionHours`: Maximum play time before break
- `breakIntervalMinutes`: Regular break intervals
- `breakDurationMin/Max`: Break duration range
- `simulateAFK`: AFK behavior during breaks

## 🔒 Anti-Detection Mechanisms

### Pattern Breaking
- Monitors action history for repetitive patterns
- Automatically introduces variations when detected
- Random action insertions (inventory checks, map opens)

### Fatigue Simulation
- Performance degrades over time
- Reaction times increase with fatigue
- More mistakes when "tired"
- Recovers during breaks

### GM Detection
- Monitors for GM indicators
- Immediate cessation of automation
- Switches to AFK mode
- Extended break period

### Input Method Variation
- **SendInput**: Standard Windows input (more detectable)
- **PostMessage**: Window message posting (less detectable)
- **DirectInput**: Hardware-level simulation (most advanced)

## 📊 Performance Profiles

### Beginner (0.2 skill)
- 250-500ms reaction time
- 15% mistake chance
- Slow mouse movements
- Frequent pauses

### Intermediate (0.6 skill)
- 120-300ms reaction time
- 3% mistake chance
- Normal mouse speed
- Balanced behavior

### Advanced (0.9 skill)
- 100-250ms reaction time
- 1% mistake chance
- Fast, precise movements
- Efficient patterns

## 🔧 Troubleshooting

### BDO Window Not Found
- Ensure Black Desert is running
- Run as administrator for memory reading
- Check window title matches "Black Desert"

### Compilation Errors
- Verify C++17 support: `-std=c++17`
- Link user32.lib: `-luser32`
- Include filesystem: `-lstdc++fs`

### Input Not Working
- Check if BDO is in focus
- Try different input methods
- Disable anti-virus temporarily

## ⚠️ Disclaimer

This software is for educational purposes only. Using automation tools in online games may violate Terms of Service and result in account penalties. The developers assume no responsibility for any consequences of using this software.

## 🤝 Contributing

Contributions are welcome! Please consider:
- Adding new class rotations
- Improving pattern detection algorithms
- Enhancing GM detection methods
- Optimizing performance

## 📝 License

This project is provided as-is for educational purposes. Use at your own risk.

## 📞 Support

For issues, questions, or contributions:
- Create an issue on the repository
- Check existing documentation
- Review code comments for implementation details

---

**Note**: This system is continuously evolving. Regular updates may be needed to maintain effectiveness against new detection methods.