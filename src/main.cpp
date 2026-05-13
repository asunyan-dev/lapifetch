#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>

#include "system.hpp"

int main(int argc, char* argv[]) {


    std::string color = "\033[0;35m";
    std::string reset = "\033[0m";
    bool showArt = true;
    bool showGPU = false;

    for(int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        std::map<std::string, std::string> colors = {
            { "black", "\033[1;30m" },
            { "red", "\033[1;31m" },
            { "green", "\033[1;32m" },
            { "yellow", "\033[1;33m" },
            { "blue", "\033[1;34m" },
            { "purple", "\033[1;35m" },
            { "cyan", "\033[1;36m" },
            { "white", "\033[1;37m" },
            { "pink", "\033[38;5;213m" }
        };

        if(arg == "--no-art" || arg == "-na") {
            showArt = false;
        }
        else if(arg == "--color" || arg == "-c") {
            if(i + 1 < argc) {
                std::string chosen = argv[++i];

                if(colors.count(chosen)) {
                    color = colors[chosen];
                } else {
                    color = colors["purple"];
                }
            }
        }
        else if(arg == "--color-list" || arg == "-cl") {
            std::cout << "LapiFetch -- Color list:\n\nblack, red, green, yellow, blue, purple, cyan, white, pink\n\nNote: it is case sensitive. If you type an invalid color, it will just use purple as default\n\nUsage: lapifetch --color <color>\nYou can also use -c\nExample: lapifetch -c green" << std::endl;
            return 0;
        }
        else if(arg == "--show-gpu" || arg == "-sg") {
            showGPU = true;
        }
        else if(arg == "--help" || arg == "-h") {
            std::cout << "LapiFetch - Simple System Information Fetcher\n\n";
            std::cout << "Arguments list:\n\n";
            std::cout << "-c,  --color <color>  |  Display with one of the valid colors.\n";
            std::cout << "-cl, --color-list     |  Display the list of valid colors.\n";
            std::cout << "-h,  --help           |  Display this help.\n";
            std::cout << "-na, --no-art         |  Display without the bunny art.\n";
            std::cout << "-sg, --show-gpu       |  Display the GPU information.\n";
            std::cout << "-v,  --version        |  Display package version.\n";

            return 0;
        }
        else if(arg == "--version" || arg == "-v") {
            std::cout << "lapifetch v0.4.0" << std::endl;
            return 0;
        }
    }

    std::vector<std::string> bunny;

    if(showArt) {
        bunny = {
            "",
            "        \\\\",
            "   ,-~~~-\\\\_",
            "  (        .\\",
            " /@\\___(__--'"
        };
    }

    std::vector<std::string> info = {
        "",
        color + getUsername() + reset + "@" + color +  getHostname() + reset,
        "",
        color + "OS:        " + reset + getOS(),
        color + "Kernel:    " + reset + getKernel(),
        color + "Uptime:    " + reset + getUptime(),
        color + "Packages:  " + reset + getPackages(),
        color + "CPU:       " + reset + getCPU()
    };

    info.at(2) = color + std::string(getHostname().size() + getUsername().size() + 1, '-') + reset;

    

    if(showGPU) {
        std::vector<std::string> gpus = getGPU();

        for(size_t i = 0; i < gpus.size(); i++) {
            info.push_back(color + "GPU:       " + reset + gpus[i]);
        }
    }

    info.push_back(color + "DE/WM:     " + reset + getDE() + " (" + getDisplayServer() + ")");
    info.push_back(color + "Terminal:  " + reset + getTerminal());
    info.push_back(color + "Shell:     " + reset + getShell());
    info.push_back(color + "RAM:       " + reset + getRAM());
    info.push_back(color + "Swap:      " + reset + getSwap());
    info.push_back(color + "Root:      " + reset + getRootStorage());

    size_t maxLines = std::max(bunny.size(), info.size());

    size_t bunnyWidth = 0;

    for(const auto& line : bunny) {
        bunnyWidth = std::max(bunnyWidth, line.length());
    }

    for(size_t i = 0; i < maxLines; i++) {
        size_t currentWidth = 0;
        if(i < bunny.size()) {
            std::cout << color << bunny[i] << reset;
            currentWidth = bunny[i].length();
        }

        std::cout << std::string(bunnyWidth - currentWidth + 5, ' ');

        if(i < info.size()) {

            std::cout << info[i];
        }

        std::cout << "\n";
    }

    return 0;
}