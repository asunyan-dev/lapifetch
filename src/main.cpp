#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include "system.hpp"

int main(int argc, char* argv[]) {


    std::string color = "\033[35m";
    std::string reset = "\033[0m";
    bool showArt = true;
    bool showGPU = false;

    for(int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if(arg == "--no-art" || arg == "-na") {
            showArt = false;
        }
        else if(arg == "--color" || arg == "-c") {
            if(i + 1 < argc) {
                std::string chosen = argv[++i];

                if(chosen == "pink") {
                    color = "\033[38;5;213m";
                }
                else if(chosen == "blue") {
                    color = "\033[34m";
                }
                else if(chosen == "green") {
                    color = "\033[32m";
                }
                else if(chosen == "purple") {
                    color = "\033[35m";
                }
                else if(chosen == "red") {
                    color = "\033[31m";
                } else {
                    color = "\033[35m";
                }
            }
        }
        else if(arg == "--color-list" || arg == "-cl") {
            std::cout << R"(
            LapiFetch -- Color list:
            - pink
            - blue
            - green
            - purple
            - red

            Usage: lapifetch --color <color>
            You can also use -c
            Example: lapifetch -c green
            )";
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
            std::cout << "lapifetch v0.2.0";
            return 0;
        }
    }

    std::vector<std::string> bunny;

    if(showArt) {
        bunny = {
            "        \\\\",
            "   ,-~~~-\\\\_",
            "  (        .\\",
            " /@\\___(__--'"
        };
    }

    std::vector<std::string> info = {
        getUsername() + "@" + getHostname(),
        color + "------------" + reset,
        color + "OS:        " + reset + getOS(),
        color + "Kernel:    " + reset + getKernel(),
        color + "Uptime:    " + reset + getUptime(),
        color + "Packages:  " + reset + getPackages(),
        color + "CPU:       " + reset + getCPU()
    };

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