// ============================================================================
//  test_main.cpp  -  Runs every MAZE-X test. Exit code 0 = all passed.
// ============================================================================

#include <cstring>
#include "ConsoleView.h"
#include "TestUtil.h"

void runMazeTests();
void runSolverTests();
void runEngineTests();

int main(int argc, char* argv[]) {
    setupConsole();
    if (argc > 1 && std::strcmp(argv[1], "-q") == 0) g_verbose = false;   // -q: only show failures

    runMazeTests();
    runSolverTests();
    runEngineTests();

    std::cout << "\n" << (g_failures == 0 ? "\033[92m" : "\033[91m")
              << (g_checks - g_failures) << "/" << g_checks << " checks passed\033[0m\n";
    return g_failures == 0 ? 0 : 1;
}
