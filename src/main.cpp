#include <cstddef>
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
    bool showGPU = true;
    std::vector<std::string> info;
    bool showColor = true;
    bool compactMode = false;

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
        else if(arg == "--no-gpu" || arg == "-ng") {
            showGPU = false;
        }
        else if(arg == "--help" || arg == "-h") {
            std::cout << "LapiFetch - Simple System Information Fetcher\n\n";
            std::cout << "Arguments list:\n\n";
            std::cout << "-c,  --color <color>  |  Display with one of the valid colors.\n";
            std::cout << "-cl, --color-list     |  Display the list of valid colors.\n";
            std::cout << "-cm, --compact-mode   |  Display info in a compact mode.\n";
            std::cout << "-h,  --help           |  Display this help.\n";
            std::cout << "-na, --no-art         |  Display without the bunny art.\n";
            std::cout << "-nc, --no-color       |  Display without any color.\n";
            std::cout << "-ng, --no-gpu         |  Hide the GPU information.\n";
            std::cout << "-v,  --version        |  Display package version.\n";

            return 0;
        }
        else if(arg == "--version" || arg == "-v") {
            std::cout << "lapifetch v0.6.0" << std::endl;
            return 0;
        }
        else if(arg == "--no-color" || arg == "-nc") {
            showColor = false;
        }
        else if(arg == "--compact-mode" || arg == "-cm") {
            compactMode = true;
        }
    }

    std::vector<std::string> infoStrings = {
        getOS(),
        getKernel(),
        getUptime(),
        getPackages(),
        getCPU()
    };

    if(showGPU) {
        std::vector<std::string> gpus = getGPU();

        for(size_t i = 0; i < gpus.size(); i++) {
            infoStrings.push_back(gpus[i]);
        }
    }

    infoStrings.push_back(getDE());
    infoStrings.push_back(getTerminal());
    infoStrings.push_back(getShell());
    infoStrings.push_back(getRAM());
    infoStrings.push_back(getSwap());
    infoStrings.push_back(getRootStorage());

    std::vector<std::string> fullLabels = {
        "OS:        ",
        "Kernel:    ",
        "Uptime:    ",
        "Packages:  ",
        "CPU:       "
    };

    if(showGPU) {
        std::vector<std::string> gpus = getGPU();
        for(size_t i = 0; i < gpus.size(); i++) {
            fullLabels.push_back("GPU:       ");
        }
    }

    fullLabels.push_back("DE/WM:     ");
    fullLabels.push_back("Terminal:  ");
    fullLabels.push_back("Shell:     ");
    fullLabels.push_back("RAM:       ");
    fullLabels.push_back("Swap:      ");
    fullLabels.push_back("Root:      ");

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

    info.push_back("");
    if(showColor) {
        info.push_back(color + getUsername() + reset + "@" + color + getHostname() + reset);
    } else {
        info.push_back(getUsername() + "@" + getHostname());
    }

    info.push_back(std::string(getHostname().size() + getUsername().size() + 1, '-'));

    if(compactMode) {
        for(size_t i = 0; i < infoStrings.size(); i++) {
            if(showColor) {
                info.push_back(color + "-" + reset + "  " + infoStrings[i]);
            } else {
                info.push_back("-  " + infoStrings[i]);
            }
        }
    } else {
        for(size_t i = 0; i < infoStrings.size(); i++) {
            if(showColor) {
                info.push_back(color + fullLabels[i] + reset + infoStrings[i]);
            } else {
                info.push_back(fullLabels[i] + infoStrings[i]);
            }
        }
    }

    info.push_back("");

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