#pragma once
#include <string>

namespace FileDialog {
    // Initializes the native file dialog
    void init();
    
    // Quits the native file dialog
    void quit();

    // Opens a folder picker dialog. Returns empty string if canceled or failed.
    std::string pickFolder();
}
