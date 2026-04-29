#include "file_dialog.hpp"
#include <nfd.hpp>
#include <iostream>

namespace FileDialog {

    void init() {
        NFD::Init();
    }

    void quit() {
        NFD::Quit();
    }

    std::string pickFolder() {
        NFD::UniquePathU8 outPath;
        nfdresult_t result = NFD::PickFolder(outPath);

        if (result == NFD_OKAY) {
            return std::string(outPath.get());
        } else if (result == NFD_CANCEL) {
            std::cout << "User pressed cancel." << std::endl;
        } else {
            std::cout << "Error: " << NFD::GetError() << std::endl;
        }

        return "";
    }
}
