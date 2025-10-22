================================================================================
                    BDO SECURE KERNEL - SIMPLE README
================================================================================

WHAT THIS IS:
  Kernel driver to read Black Desert memory while bypassing Xigncode3

WHAT YOU HAVE:
  - BDO_SecureKernel.sys (custom KMDF driver)
  - BDO_SecureClient library (to use the driver)
  - RTCore64 scanner (uses existing RTCore64.sys)
  - Auto address finder (replaces Cheat Engine)

================================================================================
OPTION 1: Use RTCore64 (Existing Driver - NO TEST SIGNING)
================================================================================

If you have MSI Afterburner installed:

1. compile_all.bat
2. BDO_AddressScanner_RTCore.exe
3. Done

No custom driver needed!

================================================================================
OPTION 2: Use Custom Driver (Better, Needs Test Signing)
================================================================================

1. BUILD_EVERYTHING_SECURE.bat
2. enable_test_signing.bat (restart required)
3. install_driver.bat
4. BDO_AutoAddressFinder.exe

================================================================================
FINDING BDO ADDRESSES (NO CHEAT ENGINE)
================================================================================

Xigncode3 blocks Cheat Engine, so use the scanners:

With RTCore64:
  BDO_AddressScanner_RTCore.exe
  
With Custom Driver:
  BDO_AutoAddressFinder.exe

Both scan for patterns and save to BDO_FOUND_ADDRESSES.txt

================================================================================
USING THE BOT
================================================================================

1. Get addresses from BDO_FOUND_ADDRESSES.txt
2. Update BDO_PracticalBot.cpp line 128
3. compile_all.bat
4. BDO_PracticalBot.exe

================================================================================
FILES YOU ACTUALLY NEED
================================================================================

Driver:
  BDO_SecureKernel.h/c/inf/vcxproj
  BDO_Stealth.c

Client:
  BDO_SecureClient.h/cpp

Programs:
  BDO_KernelTest.cpp (test driver)
  BDO_AutoAddressFinder.cpp (find addresses with custom driver)
  BDO_AddressScanner_RTCore.cpp (find addresses with RTCore64)
  BDO_PracticalBot.cpp (health bot)

Scripts:
  BUILD_EVERYTHING_SECURE.bat
  compile_all.bat
  install_driver.bat
  enable_test_signing.bat

Helpers:
  COMPLETE_WORKING_SOLUTION.bat (tests what you have)
  CRITICAL_ISSUES_FOUND.txt (known issues)
  TODO_TO_MAKE_IT_WORK.txt (checklist)

================================================================================
QUICK START
================================================================================

Run:
  COMPLETE_WORKING_SOLUTION.bat

This checks what you have and tells you what to do next.

================================================================================

