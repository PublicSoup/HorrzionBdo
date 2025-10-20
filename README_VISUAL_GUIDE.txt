================================================================================
                    BDO BOT - VISUAL QUICK START GUIDE
================================================================================

┌─────────────────────────────────────────────────────────────────────────────┐
│                          STEP 1: COMPILE (2 MIN)                            │
└─────────────────────────────────────────────────────────────────────────────┘

    Option A: Easy Way
    ┌──────────────────────────┐
    │  Double-click:           │
    │  COMPILE_NOW.bat         │    Wait for "✓ COMPILATION SUCCESSFUL!"
    └──────────────────────────┘

    Option B: Visual Studio
    ┌──────────────────────────────────────────────────────────────┐
    │  1. Open: BDOStealthBot.sln                                  │
    │  2. Set: Release | x64 (top toolbar)                         │
    │  3. Press: Ctrl + Shift + B                                  │
    │  4. Wait: "Build succeeded"                                  │
    └──────────────────────────────────────────────────────────────┘

    ✓ Output: bin\x64\Release\BDOStealthBot.exe


┌─────────────────────────────────────────────────────────────────────────────┐
│                   STEP 2: FIND HEALTH ADDRESS (5 MIN)                       │
└─────────────────────────────────────────────────────────────────────────────┘

    1. Start Black Desert Online → Log into character
    2. Download Cheat Engine: www.cheatengine.org
    3. Open Cheat Engine

    ┌─────────────────────────────────────────────────────────────┐
    │ Cheat Engine                                           [ _ □ X]│
    │ ┌─────────────────────────────────────────────────────────┐ │
    │ │ Select Process: BlackDesert64.exe                       │ │
    │ ├─────────────────────────────────────────────────────────┤ │
    │ │ Value: [1500____] [First Scan]                          │ │
    │ │ Scan Type: Exact Value                                  │ │
    │ │ Value Type: 4 Bytes                                     │ │
    │ ├─────────────────────────────────────────────────────────┤ │
    │ │ Found: 125,431 addresses                                │ │
    │ └─────────────────────────────────────────────────────────┘ │
    └─────────────────────────────────────────────────────────────┘

    4. Take damage in game (health drops to 1200)
    5. Enter 1200, click [Next Scan]
    6. Repeat until you have 1-5 results

    ┌─────────────────────────────────────────────────────────────┐
    │ Results:                                                      │
    │ ┌─────────────┬─────────┬──────────┐                        │
    │ │   Address   │  Value  │   Type   │                        │
    │ ├─────────────┼─────────┼──────────┤                        │
    │ │ 7FF6A2B4C890│  1200   │ 4 Bytes  │ ← Your health address! │
    │ │ 7FF6A2B4C894│  1500   │ 4 Bytes  │ ← Max health           │
    │ └─────────────┴─────────┴──────────┘                        │
    └─────────────────────────────────────────────────────────────┘

    ✓ Write down: 7FF6A2B4C890


┌─────────────────────────────────────────────────────────────────────────────┐
│                      STEP 3: RUN THE BOT (3 MIN)                            │
└─────────────────────────────────────────────────────────────────────────────┘

    1. Right-click: BDOStealthBot.exe → Run as Administrator

    ┌─────────────────────────────────────────────────────────────┐
    │ BDO Bot Memory Integration Example                          │
    │ ════════════════════════════════════════════════════        │
    │                                                              │
    │ Successfully attached to BlackDesert64.exe                  │
    │                                                              │
    │ === Bot Control Menu ===                                    │
    │ 1. Scan for addresses automatically                         │
    │ 2. Enter addresses manually                                 │
    │ 3. Load addresses from file                                 │
    │ 4. Start Combat Bot                                         │
    │ 5. Start Fishing Bot                                        │
    │ 7. Monitor values only                                      │
    │ 8. Save current addresses                                   │
    │ 0. Exit                                                     │
    │                                                              │
    │ Choice: _                                                   │
    └─────────────────────────────────────────────────────────────┘

    2. Type: 2 (Enter addresses manually)

    ┌─────────────────────────────────────────────────────────────┐
    │ === Manual Address Input ===                                │
    │ Enter addresses in hexadecimal (e.g., 7FF6A2B4C890)         │
    │                                                              │
    │ Player Health Address: 0x7FF6A2B4C890_                      │
    │ Player Mana Address: 0x [Enter to skip]                     │
    │ ...                                                          │
    │                                                              │
    │ ✓ Set player health address to 0x7FF6A2B4C890              │
    └─────────────────────────────────────────────────────────────┘

    3. Test with option: 7 (Monitor values)

    ┌─────────────────────────────────────────────────────────────┐
    │ === BDO Memory Values ===                                   │
    │ Health: 1500/1500 (100.0%)    ← Should match game!         │
    │ Mana: [Not Available]                                       │
    │ Position: [Not Available]                                   │
    │ Combat: Not in Combat                                       │
    │                                                              │
    │ Press Ctrl+C to stop monitoring...                          │
    └─────────────────────────────────────────────────────────────┘

    4. If values are correct, choose: 4 (Start Combat Bot)

    ┌─────────────────────────────────────────────────────────────┐
    │ Setting up combat actions...                                │
    │ Registered action: UseHealthPotion                          │
    │ Registered action: UseManaPotion                            │
    │ Registered action: BasicAttack                              │
    │ Added trigger: LowHealthTrigger                             │
    │                                                              │
    │ *** BOT IS NOW RUNNING ***                                  │
    │ Press Enter to stop...                                      │
    │                                                              │
    │ [COMBAT] Using health potion (Key 1)  ← Bot in action!     │
    └─────────────────────────────────────────────────────────────┘


┌─────────────────────────────────────────────────────────────────────────────┐
│                         WHAT THE BOT DOES                                   │
└─────────────────────────────────────────────────────────────────────────────┘

    ┌────────────────────────┐
    │  Bot Monitoring Loop   │
    │  (Every 100ms)         │
    └───────────┬────────────┘
                │
                ▼
    ┌───────────────────────────────────────────────────────────┐
    │ Is Health < 50%?                                          │
    ├───────────────────────────────────────────────────────────┤
    │ YES → Press '1' (Health Potion)                           │
    │       Wait 5 seconds (cooldown)                           │
    │                                                            │
    │ NO  → Continue monitoring                                 │
    └───────────────────────────────────────────────────────────┘
                │
                ▼
    ┌───────────────────────────────────────────────────────────┐
    │ Is Mana < 30%?                                            │
    ├───────────────────────────────────────────────────────────┤
    │ YES → Press '2' (Mana Potion)                             │
    │       Wait 5 seconds (cooldown)                           │
    │                                                            │
    │ NO  → Continue monitoring                                 │
    └───────────────────────────────────────────────────────────┘
                │
                ▼
    ┌───────────────────────────────────────────────────────────┐
    │ In Combat?                                                │
    ├───────────────────────────────────────────────────────────┤
    │ YES → Left Click (Attack)                                 │
    │       Use Skills Q, E, R                                  │
    │                                                            │
    │ NO  → Continue monitoring                                 │
    └───────────────────────────────────────────────────────────┘


┌─────────────────────────────────────────────────────────────────────────────┐
│                           TROUBLESHOOTING                                   │
└─────────────────────────────────────────────────────────────────────────────┘

    Problem: "Failed to attach to game"
    ┌────────────────────────────────────────────────────────────┐
    │ Solution:                                                   │
    │ • Make sure BDO is running                                 │
    │ • Run bot as Administrator                                 │
    │ • Log into a character (not at character select)          │
    └────────────────────────────────────────────────────────────┘

    Problem: "Health not reading correctly"
    ┌────────────────────────────────────────────────────────────┐
    │ Solution:                                                   │
    │ • Find address again with Cheat Engine                     │
    │ • Addresses change after game restart                      │
    │ • Make sure you entered address without "0x"              │
    │   (Type: 7FF6A2B4C890 not 0x7FF6A2B4C890)                 │
    └────────────────────────────────────────────────────────────┘

    Problem: "Bot not pressing keys"
    ┌────────────────────────────────────────────────────────────┐
    │ Solution:                                                   │
    │ • Run game in Windowed mode (not Fullscreen)              │
    │ • Make sure potion is in slot 1                           │
    │ • Check health actually dropped below 50%                  │
    │ • Try changing threshold to 0.8 (80%) for testing         │
    └────────────────────────────────────────────────────────────┘


┌─────────────────────────────────────────────────────────────────────────────┐
│                         FILE LOCATIONS                                      │
└─────────────────────────────────────────────────────────────────────────────┘

    Project Root:
    C:\Users\NAS_ADMIN\source\repos\BDO BOT\HorrzionBdo\

    Compiled Bot:
    └─ bin\x64\Release\BDOStealthBot.exe  ← Run this

    Source Code:
    ├─ BDO_BotExample.cpp                  ← Main program
    ├─ BDO_BotMemoryIntegration.cpp        ← Integration system
    └─ BDO_MemoryScanner.cpp               ← Scanner engine

    Documentation:
    ├─ START_HERE.md                       ← Quick start
    ├─ STEP_BY_STEP_GUIDE.md              ← Detailed guide
    ├─ QUICK_START.md                      ← 5-min guide
    └─ PROJECT_SUMMARY.md                  ← Overview


┌─────────────────────────────────────────────────────────────────────────────┐
│                         KEYBOARD SHORTCUTS                                  │
└─────────────────────────────────────────────────────────────────────────────┘

    In Bot:
    [1-9]    → Menu options
    [Enter]  → Confirm / Stop bot
    [Ctrl+C] → Emergency stop

    In Visual Studio:
    [Ctrl+Shift+B] → Build solution
    [F5]           → Run with debugging
    [Ctrl+F5]      → Run without debugging

    In Game:
    [1]      → Health potion (default)
    [2]      → Mana potion (default)
    [Q,E,R]  → Skills (default)


┌─────────────────────────────────────────────────────────────────────────────┐
│                         EXAMPLE ADDRESSES                                   │
└─────────────────────────────────────────────────────────────────────────────┘

    Example addresses (YOURS WILL BE DIFFERENT!):

    Player Health:     7FF6A2B4C890
    Player Max Health: 7FF6A2B4C894
    Player Mana:       7FF6A2B4C898
    Player Max Mana:   7FF6A2B4C89C
    Player X Position: 7FF6A2B4C8A0
    Player Y Position: 7FF6A2B4C8A4
    Player Z Position: 7FF6A2B4C8A8
    Combat State:      7FF6A2B4C8B0

    NOTE: These are examples only!
    You must find your own addresses with Cheat Engine.


┌─────────────────────────────────────────────────────────────────────────────┐
│                         SUCCESS CHECKLIST                                   │
└─────────────────────────────────────────────────────────────────────────────┘

    Before running bot:
    [✓] Visual Studio installed
    [✓] Bot compiled (COMPILE_NOW.bat)
    [✓] BDO running and logged in
    [✓] Health address found (Cheat Engine)
    [✓] Health potion in slot 1
    [✓] Bot running as Administrator
    [✓] Game in Windowed mode

    Testing:
    [✓] Monitoring shows correct health
    [✓] Taking damage updates value
    [✓] Health drops below 50%
    [✓] Bot presses '1' automatically
    [✓] Potion is used in game
    [✓] Bot has 5-second cooldown

    SUCCESS! Your bot is working! 🎉


┌─────────────────────────────────────────────────────────────────────────────┐
│                         NEXT STEPS                                          │
└─────────────────────────────────────────────────────────────────────────────┘

    Once basic auto-potion works:

    1. Find more addresses:
       • Mana address
       • Position (X, Y, Z)
       • Combat state

    2. Add more actions:
       • Skills (Q, E, R)
       • Buff potions
       • Food/elixirs

    3. Save your addresses:
       • Option 8 in menu
       • Create addresses.txt
       • Load next time with Option 3

    4. Customize bot:
       • Change thresholds
       • Adjust cooldowns
       • Add custom triggers

    5. Advanced features:
       • Fishing automation
       • Gathering automation
       • Combat rotations


┌─────────────────────────────────────────────────────────────────────────────┐
│                         GETTING HELP                                        │
└─────────────────────────────────────────────────────────────────────────────┘

    For more information, read:

    Quick Start:
    • START_HERE.md           - You are here
    • QUICK_START.md          - 5-minute guide

    Detailed Guides:
    • STEP_BY_STEP_GUIDE.md   - Complete walkthrough
    • GUIDE_MemoryIntegration.md - Advanced integration

    Reference:
    • PROJECT_SUMMARY.md      - Project overview
    • CODEBASE_ANALYSIS.md    - Technical details
    • TROUBLESHOOTING.md      - Common issues


================================================================================
                           🎉 YOU'RE READY TO GO! 🎉
================================================================================

                    1. Double-click: COMPILE_NOW.bat
                    2. Find health address in Cheat Engine
                    3. Run bot as Administrator
                    4. Start automation!

                         Happy Botting! 🚀

================================================================================
