#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <functional>
#include <unordered_map>

#include "system.hpp"

int main(int argc, char* argv[]) {


    std::string color;
    std::string reset = "\033[0m";
    bool showArt = true;
    bool showGPU = true;
    std::vector<std::string> info;
    bool showColor = true;
    bool compactMode = false;
    bool essential = false;
    bool customOrder = false;
    bool showIP = true;
    std::vector<std::string> order;
    std::unordered_map<std::string, std::function<std::string()>> infos = {
        { "os", getOS },
        { "kernel", getKernel },
        { "uptime", getUptime },
        { "packages", getPackages },
        { "cpu", getCPU },
        { "gpu", getCPU },
        { "de", [] { return getDE() + " (" + getDisplayServer() + ")"; } },
        { "terminal", getTerminal },
        { "shell", getShell },
        { "ram", getRAM },
        { "swap", getSwap },
        { "root", getRootStorage },
        { "[separator]", makeSeparator },
        { "display", getDisplay },
        { "ip", getLocalIP }
    };
    

    std::unordered_map<std::string, std::string> colors = {
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
    

    for(int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if(arg == "--no-art" || arg == "-na") {
            showArt = false;
        }
        else if(arg == "--color-info" || arg == "-ci") {
            std::cout << "lapifetch - color config\n\n";
            std::cout << "The default color is purple.\nTo config a color for lapifetch, you have to generate the config files if they don't exist and then edit the color file. To do so, do the following:\n\n$lapifetch --gen-config\n$nano ~/.config/lapifetch/color\n\nNow replace purple with the color you want, lowercase and without spaces. Here is the list of valid colors:\n\n";
            for(auto color : colors) {
                std::cout << "- " << color.second << color.first << reset << "\n";
            }
            std::cout << "\n";
            return 0;
        }
        else if(arg == "--help" || arg == "-h") {
            std::cout << "LapiFetch - Simple System Information Fetcher\n\n";
            std::cout << "Arguments list:\n\n";
            std::cout << "-ci, --color-info     |  Display info for color config.\n";
            std::cout << "-cm, --compact-mode   |  Display info in a compact mode.\n";
            std::cout << "-dc, --delete-config  |  Remove config directory and files.\n";
            std::cout << "-e,  --essential      |  Display a short version of lapifetch.\n";
            std::cout << "-gc, --gen-config     |  Check and generate config files if they don't exist.\n";
            std::cout << "-h,  --help           |  Display this help.\n";
            std::cout << "-na, --no-art         |  Display without the bunny art.\n";
            std::cout << "-nc, --no-color       |  Display without any color.\n";
            std::cout << "-ni, --no-ip          |  Display without showing local IP.\n";
            std::cout << "-oi, --order-info     |  Display the info for order file.\n";
            std::cout << "-v,  --version        |  Display package version.\n";

            return 0;
        }
        else if(arg == "--version" || arg == "-v") {
            std::cout << "lapifetch v1.2.0" << std::endl;
            return 0;
        }
        else if(arg == "--no-gpu" || arg == "-ng") {
            showGPU = false;
        }
        else if(arg == "--no-color" || arg == "-nc") {
            showColor = false;
        }
        else if(arg == "--compact-mode" || arg == "-cm") {
            compactMode = true;
        }
        else if(arg == "--essential" || arg == "-e") {
            essential = true;
        }
        else if(arg == "--order-info" || arg == "-oi") {
            std::cout << "lapifetch - order config\n\n";
            std::cout << "Use lapifetch --gen-config to check if config path exists.\n\n";
            std::cout << "use nano to edit the file \"order\":\n$nano ~/.config/lapifetch/order\n\n";
            std::cout << "Now put the order you want, one option per line, lowercase, and no spaces. Here is the list of valid info:\n\n";
            for(auto info : infos) {
                std::cout << "- " << info.first << "\n";
            }
            std::cout << "[separator] lets you add a separator between two infos. It will show a line of ' - ' characters.\n ";
            return 0;
        }
        else if(arg == "--gen-config" || arg == "-gc") {
            std::filesystem::path config_dir = std::filesystem::path(std::getenv("HOME")) / ".config" / "lapifetch";

            if(!std::filesystem::exists(config_dir)) {
                std::filesystem::create_directories(config_dir);
                std::cout << "===> Created config folder.\n";
            }
            else {
                std::cout << "===> Config folder exists.\n";
            }

            std::filesystem::path colorPath = std::filesystem::path(config_dir) / "color";

            if(!std::filesystem::exists(colorPath)) {
                std::ofstream color_file(colorPath);

                if(color_file.is_open()) {
                    color_file << "purple";
                }

                color_file.close();

                std::cout << "===> Created color file.\n";
            }
            else {
                std::cout << "===> Color file exists.\n";
            }

            std::filesystem::path orderPath = std::filesystem::path(config_dir) / "order";

            if(!std::filesystem::exists(orderPath)) {
                std::ofstream order_file(orderPath);

                if(order_file.is_open()) {
                    order_file <<
                        "os\n"
                        "kernel\n"
                        "uptime\n"
                        "packages\n"
                        "cpu\n"
                        "gpu\n"
                        "display\n"
                        "de\n"
                        "terminal\n"
                        "shell\n"
                        "ram\n"
                        "swap\n"
                        "root\n"
                        "ip\n";
                }

                order_file.close();

                std::cout << "===> Created order file.\n";
            }
            else {
                std::cout << "===> Order file exists.\n";
            }

            std::cout << "\n\nConfig files checked!\n";
            return 0;
        }
        else if(arg == "--no-ip" || arg == "-ni") {
            showIP = false;
        }

        else if(arg == "--delete-config" || arg == "-dc") {
            std::filesystem::path configPath = std::filesystem::path(std::getenv("HOME")) / ".config/lapifetch";

            if(std::filesystem::exists(configPath)) {
                std::filesystem::remove_all(configPath);

                std::cout << "Deleted " + configPath.string() + " successfully.\n";
            } else {
                std::cout << "Path to lapifetch config does not exists.\n";
            }

            return 0;
        }
    }

    std::filesystem::path orderPath = std::filesystem::path(std::getenv("HOME")) / ".config/lapifetch/order";

    std::filesystem::path colorPath = std::filesystem::path(std::getenv("HOME")) / ".config/lapifetch/color";

    if(std::filesystem::exists(colorPath)) {
        std::ifstream file(colorPath);
        std::string line;
        while(std::getline(file, line)) {
            if(colors.count(line)) {
                color = colors[line];
            } else {
                std::cout << "Error: Invalid color. See lapifetch -cl and edit ~/.config/lapifetch/color to fix it.";
                return 0;
            }
        }
        file.close();
    } else {
        color = colors["purple"];
    }

    if(std::filesystem::exists(orderPath)) {
        customOrder = true;
    }

    if(customOrder) {
        std::ifstream file(orderPath);
        std::string line;
        while(std::getline(file, line)) {
            if(!infos.count(line)) {
                std::cout << "Error: " << line << " is not a valid option.\nFix it in ~/.config/lapifetch/order";
                return 0;
            }
            order.push_back(line);
        }
        file.close();
    }

    if(!customOrder) {
        order = {
            "os",
            "kernel",
            "uptime",
            "packages",
            "cpu",
            "gpu",
            "display",
            "de",
            "terminal",
            "shell",
            "ram",
            "swap",
            "root",
            "ip"
        };
    }

    std::vector<std::string> infoStrings;
    std::vector<std::string> cachedGPUs;

    if(showGPU) {
        cachedGPUs = getGPU();
    }

    if(essential) {
        infoStrings.push_back(getOS());
        infoStrings.push_back(getKernel());
        infoStrings.push_back(getUptime());
        infoStrings.push_back(getCPU());
        infoStrings.push_back(getDE() + " (" + getDisplayServer() + ")");
        infoStrings.push_back(getTerminal() + " (" + getShell() + ")");
    }
    else {
        for(size_t i = 0; i < order.size(); i++) {
            if(order[i] == "ip" && !showIP) {
                continue;
            }

            if(order[i] == "gpu") {
                for(size_t j = 0; j < cachedGPUs.size(); j++) {
                    infoStrings.push_back(cachedGPUs[j]);
                }

                continue;
            }

            if(order[i] == "[separator]") {
                if(compactMode) {
                    continue;
                } else {
                    infoStrings.push_back("");
                    continue;
                }
            }

            infoStrings.push_back(infos[order[i]]());
        }
    }

    if(essential) {
        showColor = false;
        compactMode = true;
    }

    std::unordered_map<std::string, std::string> labels = {
        { "os", "OS:        " },
        { "kernel", "Kernel:    " },
        { "uptime", "Uptime:    " },
        { "packages", "Packages:  " },
        { "cpu", "CPU:       " },
        { "gpu", "gpu" },
        { "display", "Display:   " },
        { "de", "DE/WM:     "},
        { "terminal", "Terminal:  " },
        { "shell", "Shell:     " },
        { "ram", "RAM:       " },
        { "swap", "Swap:      " },
        { "root", "Root:      " },
        { "[separator]", makeSeparator() },
        { "ip", "IP:        " }
    };

    std::vector<std::string> fullLabels;

    for(size_t i = 0; i < order.size(); i++) {
        if(order[i] == "ip" && !showIP) {
            continue;
        }

        if(order[i] == "gpu") {
            for(size_t j = 0; j < cachedGPUs.size(); j++) {
                if(showColor) {
                    fullLabels.push_back(color + "GPU:       " + reset);
                } else {
                    fullLabels.push_back("GPU:       ");
                }
            }
            continue;
        }
        if(order[i] == "[separator]") {
            if(compactMode) {
                continue;
            } else {
                fullLabels.push_back(makeSeparator());
                continue;
            }
        }

        if(showColor) {
            fullLabels.push_back(color + labels[order[i]] + reset);
        } else {
            fullLabels.push_back(labels[order[i]]);
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
            info.push_back(fullLabels[i] + infoStrings[i]);
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