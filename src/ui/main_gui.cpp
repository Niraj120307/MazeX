// ============================================================================
//  main_gui.cpp  -  Entry point of the SFML application (MazeX.exe)
// ============================================================================

#include <exception>
#include <iostream>
#include "App.h"

int main() {
    try {
        App app;
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "MAZE-X failed to start: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
