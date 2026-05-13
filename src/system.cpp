#include "system.hpp"

#include <iomanip>
#include <ios>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/statvfs.h>

#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <array>
#include <cstdio>
#include <vector>
#include <algorithm>
#include <iomanip>

std::string getUsername() {
    const char* user = getenv("USER");

    if(user) {
        return std::string(user);
    } else {
        return "unknown";
    }
}

std::string getHostname() {
    char hostname[256];

    gethostname(hostname, sizeof(hostname));

    return std::string(hostname);
}

std::string getKernel() {
    struct utsname buffer;

    uname(&buffer);

    return std::string(buffer.release);
}

std::string getOS() {
    std::ifstream file ("/etc/os-release");

    std::string line;

    while(std::getline(file, line)) {
        if(line.find("PRETTY_NAME") == 0) {
            std::string value = line.substr(13);

            if(value.back() == '"') {
                value.pop_back();
            }

            return value;
        }
    }

    return "Unknown";
}

std::string getCPU() {
    std::ifstream file("/proc/cpuinfo");

    std::string line;

    while(std::getline(file, line)) {
        if(line.find("model name") != std::string::npos) {
            size_t pos = line.find(":");

            if(pos != std::string::npos) {
                return line.substr(pos + 2);
            }
        }
    }

    return "Unknown CPU";
}

std::string formatBytes(unsigned long long bytes) {
    double gib = bytes / 1024.0 / 1024.0 / 1024.0;

    std::stringstream ss;

    ss << std::fixed << std::setprecision(1) << gib << "GiB";

    return ss.str();
}

std::string getRAM() {
    std::ifstream file("/proc/meminfo");

    std::string line;

    long total = 0;
    long available = 0;

    while(std::getline(file, line)) {
        if(line.find("MemTotal:") == 0) {
            std::stringstream ss(line);
            std:: string tmp;

            ss >> tmp >> total;
        }

        if(line.find("MemAvailable:") == 0) {
            std::stringstream ss(line);
            std::string tmp;

            ss >> tmp >> available;
        }
    }

    long used = total - available;

    int percent = static_cast<int>(
        (used * 100.0) / total
    );

    return formatBytes(used * 1024ULL) + " / " + formatBytes(total * 1024ULL) + " (" + std::to_string(percent) + "%)";
}

std::string getSwap() {
    std::ifstream file("/proc/meminfo");

    std::string line;

    long total = 0;
    long free = 0;

    while(std::getline(file, line)) {
        if(line.find("SwapTotal:") == 0) {
            std::stringstream ss(line);
            std::string tmp;

            ss >> tmp >> total;
        }

        if(line.find("SwapFree:") == 0) {
            std::stringstream ss(line);
            std::string tmp;

            ss >> tmp >> free;
        }
    }

    long used = total - free;

    int percent = 0;

    if(total > 0) {
        percent = static_cast<int>(
            (used * 100.0) / total
        );
    }

    return formatBytes(used * 1024ULL) + " / " + formatBytes(total * 1024ULL) + " (" + std::to_string(percent) + "%)";
}

std::string getRootStorage() {
    struct statvfs stat;

    if(statvfs("/", &stat) != 0) {
        return "Unknown";
    }

    unsigned long long total = stat.f_blocks * stat.f_frsize;

    unsigned long long free = stat.f_bfree * stat.f_frsize;

    unsigned long long used = total - free;

    int percent = static_cast<int>(
        (used * 100.0) / total
    );

    return formatBytes(used) + " / " + formatBytes(total) + " (" + std::to_string(percent) + "%)";
}

std::string exec(const char* cmd) {
    std::array<char, 128> buffer;

    std::string result;

    FILE* pipe = popen(cmd, "r");

    if(!pipe) {
        return "Unknown";
    }

    while(fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }

    pclose(pipe);

    return result;
}

std::vector<std::string> getGPU() {
    std::vector<std::string> gpus;
    std::string output = exec("lspci | grep -E 'VGA'");

    if(output.empty()) {
        gpus.push_back("Unknown GPU");
        return gpus;
    }

    std::stringstream ss(output);

    std::string line;

    while(std::getline(ss, line)) {
        size_t pos = line.find(": ");

        if(pos != std::string::npos) {
            line = line.substr(pos + 2);
        }

        size_t rev = line.find("(rev");

        if(rev != std::string::npos) {
            line = line.substr(0, rev);
        }

        while(!line.empty() && line.back() == ' ') {
            line.pop_back();
        }

        gpus.push_back(line);
    }

    return gpus;
}



std::string getUptime() {
    std::ifstream file("/proc/uptime");

    double uptimeSeconds;

    file >> uptimeSeconds;

    int hours = uptimeSeconds / 3600;
    int minutes = (static_cast<int>(uptimeSeconds) % 3600) / 60;

    return std::to_string(hours) + "h " + std::to_string(minutes) + "m";
}

std::string getShell() {
    const char* shell = getenv("SHELL");

    if(!shell) {
        return "Unknown";
    }

    std::string shellPath(shell);

    size_t pos = shellPath.find_last_of('/');

    if(pos != std::string::npos) {
        return shellPath.substr(pos + 1);
    }

    return shellPath;
}

std::string getTerminal() {
    const char* termProgram = getenv("TERM_PROGRAM");

    if(termProgram) {
        return std::string(termProgram);
    }

    const char* term = getenv("TERM");

    if(term) {
        return std::string(term);
    }

    return "Unknown";
}

std::string getDE() {
    const char* de = getenv("XDG_CURRENT_DESKTOP");

    if(de) {
        return std::string(de);
    }

    return "Unknown";
}

std::string getDisplayServer() {
    const char* wayland = getenv("WAYLAND_DISPLAY");

    if(wayland) {
        return "Wayland";
    }

    const char* x11 = getenv("DISPLAY");

    if(x11) {
        return "X11";
    }

    return "Unknown Display Server";
}

bool commandExists(const std::string& cmd) {
    std::string check = "which " + cmd + " > /dev/null 2>&1";

    return system(check.c_str()) == 0;
}

std::string getPackages() {
    std::vector<std::string> packages;

    if(commandExists("pacman")) {
        std::string output = exec("pacman -Qq | wc -l");

        output.erase(
            std::remove(output.begin(), output.end(), '\n'), output.end()
        );

        packages.push_back(output + " (pacman)");
    }

    if(commandExists("dpkg")) {
        std::string output = exec("dpkg-query -f '.' -W | wc -c");

        output.erase(
            std::remove(output.begin(), output.end(), '\n'), output.end()
        );

        packages.push_back(output + " (apt)");
    }

    if(commandExists("nix")) {
        std::string output = exec("nix profile list 2>/dev/null | wc -l");

        output.erase(
            std::remove(output.begin(), output.end(), '\n'),
            output.end()
        );

        int count = std::stoi(output);
        if(count > 0) {
            count--;
        }

        packages.push_back(
            std::to_string(count) + " (nix)"
        );
    }

    if(commandExists("emerge")) {
        std::string output = exec("find /var/db/pkg -mindepth 2 -maxdepth 2 -type d | wc -l");

        output.erase(
            std::remove(output.begin(), output.end(), '\n'),
            output.end()
        );

        packages.push_back(output + " (emerge)");
    }

    if(commandExists("rpm")) {
        std::string output = exec("rpm -qa | wc -l");

        output.erase(
            std::remove(output.begin(), output.end(), '\n'),
            output.end()
        );

        packages.push_back(output + " (rpm)");
    }

    if(commandExists("flatpak")) {
        std::string output = exec("flatpak list | wc -l");

        output.erase(
            std::remove(output.begin(), output.end(), '\n'), output.end()
        );

        packages.push_back(output + " (flatpak)");
    }

    if(packages.empty()) {
        return "Unknown";
    }

    std::string result;

    for(size_t i = 0; i < packages.size(); i++) {
        result += packages[i];

        if(i + 1 < packages.size()) {
            result += ", ";
        }
    }

    return result;
}