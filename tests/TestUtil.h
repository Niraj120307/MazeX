// ============================================================================
//  TestUtil.h  -  A tiny test helper (no external framework needed)
// ============================================================================

#pragma once

#include <iostream>
#include <string>

inline int g_checks   = 0;   // C++17 "inline variable": one shared copy across all test files
inline int g_failures = 0;
inline bool g_verbose = true;

inline void check(bool condition, const std::string& name) {
    ++g_checks;
    if (!condition) ++g_failures;
    if (g_verbose || !condition)
        std::cout << (condition ? "  \033[92m[PASS]\033[0m " : "  \033[91m[FAIL]\033[0m ") << name << "\n";
}

inline void section(const std::string& title) {
    std::cout << "\n\033[96m== " << title << " ==\033[0m\n";
}
