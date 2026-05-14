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
    std::vector<std::string> info;
    bool showColor = true;

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
            std::cout << "Lapifetch -- Color list:\n\n";
            for(auto color : colors) {
                std::cout << "- " << color.second << color.first << reset << "\n";
            }
            std::cout << "\n";
            std::cout << "Note: it is case sensitive. If you type an invalid color, it will just use purple as default.\n\nUsage: lapifetch --color <color>\n\nYou can also use -c\nExample: lapifetch -c green\n";
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
            std::cout << "-nc, --no-color       |  Display without any color.\n";
            std::cout << "-sg, --show-gpu       |  Display the GPU information.\n";
            std::cout << "-v,  --version        |  Display package version.\n";

            return 0;
        }
        else if(arg == "--version" || arg == "-v") {
            std::cout << "lapifetch v0.5.0" << std::endl;
            return 0;
        }
        else if(arg == "--no-color" || arg == "-nc") {
            showColor = false;
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

    if(showColor) {
        info.push_back("");
        info.push_back(color + getUsername() + "@" + getHostname() + reset);
        info.push_back(std::string(getUsername().size() + getHostname().size() + 1, '-'));
        info.push_back(color + "OS:        " + reset + getOS());
        info.push_back(color + "Kernel:    " + reset + getKernel());
        info.push_back(color + "Uptime:    " + reset + getUptime());
        info.push_back(color + "Packages:  " + reset + getPackages());
        info.push_back(color + "CPU:       " + reset + getCPU());
        if(showGPU) {
            std::vector<std::string> gpus = getGPU();

            for (size_t i = 0; i < gpus.size(); i++) {
                info.push_back(color + "GPU:       " + reset + gpus[i]);
            }
        }
        info.push_back(color + "DE/WM:     " + reset + getDE());
        info.push_back(color + "Terminal:  " + reset + getTerminal());
        info.push_back(color + "Shell:     " + reset + getShell());
        info.push_back(color + "RAM:       " + reset + getRAM());
        info.push_back(color + "Swap:      " + reset + getSwap());
        info.push_back(color + "Root:      " + reset + getRootStorage());
    }
    else if(!showColor) {
        info.push_back("");
        info.push_back(getUsername() + "@" + getHostname());
        info.push_back(std::string(getUsername().size() + getHostname().size() + 1, '-'));
        info.push_back("OS:        " + getOS());
        info.push_back("Kernel:    " + getKernel());
        info.push_back("Uptime:    " + getUptime());
        info.push_back("Packages:  " + getPackages());
        info.push_back("CPU:       " + getCPU());
        if(showGPU) {
            std::vector<std::string> gpus = getGPU();

            for(size_t i = 0; i < gpus.size(); i++) {
                info.push_back("GPU:       " + gpus[i]);
            }
        }
        info.push_back("DE/WM:     " + getDE());
        info.push_back("Terminal:  " + getTerminal());
        info.push_back("Shell:     " + getShell());
        info.push_back("RAM:       " + getRAM());
        info.push_back("Swap:      " + getSwap());
        info.push_back("Root:      " + getRootStorage());
    }

    size_t maxLines = std::max(bunny.size(), info.size());

    size_t bunnyWidth = 0;

    for(const auto& line : bunny) {
        bunnyWidth = std::max(bunnyWidth, line.length());
    }

    for(size_t i = 0; i < maxLines; i++) {
        size_t currentWidth = 0;
        if(i < bunny.size()) {

            if(!showColor) {
                std::cout << bunny[i];
            } else {
                std::cout << color << bunny[i] << reset;
            }
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