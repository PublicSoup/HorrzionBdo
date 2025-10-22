#include "pch.h"
#include "BDO_RTCore64_Driver.h"

// Forward declaration
int rtcore_test_main(int argc, char* argv[]);

// Entry point for RTCore64 test
int rtcore64_main() {
    return rtcore_test_main(0, nullptr);
}
