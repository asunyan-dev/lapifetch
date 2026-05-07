#include <iostream>
#include <string>

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

            return 0;
        }
    }

    if(showArt) {
        std::cout << color;

        std::cout << R"(
                 \\
            ,-~~~-\\_
           (        .\
         / @\___(__--'
        )";

        std::cout << reset << std::endl;
    }

    std::cout << " " + getUsername() << "@" << getHostname() << "\n";
    std::cout << "------------" << + "\n\n";

    std::cout << color + "OS:        " + reset << getOS() << "\n";
    std::cout << color + "Kernel:    " + reset << getKernel() << "\n";
    std::cout << color + "Uptime:    " + reset << getUptime() << "\n";
    std::cout << color + "CPU:       " + reset << getCPU() << "\n";
    if(showGPU) {
        std::cout << color + "GPU:       " + reset << getGPU() << "\n";
    }
    std::cout << color + "DE/WM:     " + reset << getDE() << "\n";
    std::cout << color + "Terminal:  " + reset << getTerminal() << "\n";
    std::cout << color + "Shell:     " + reset << getShell() << "\n";
    std::cout << color + "RAM:       " + reset << getRAM() << "\n";
    std::cout << color + "Swap:      " + reset << getSwap() << "\n";
    std::cout << color + "Root:      " + reset << getRootStorage() << "\n";

    return 0;
}