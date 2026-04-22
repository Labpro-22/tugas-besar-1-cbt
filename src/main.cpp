#include "gui/GuiWindow.hpp"

#include <exception>
#include <iostream>

int main() {
    try {
        GuiWindow window;
        return window.run();
    } catch (const std::exception& e) {
        std::cerr << "[FATAL] " << e.what() << "\n";
    } catch (...) {
        std::cerr << "[FATAL] Unknown exception\n";
    }
    return 1;
}
