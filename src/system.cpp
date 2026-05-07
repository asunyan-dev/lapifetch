#include "system.hpp"

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

    total /= 1024;
    used /= 1024;

    return std::to_string(used) + " MiB / " + std::to_string(total) + " MiB";
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

    total /= 1024;
    used /= 1024;

    return std::to_string(used) + " MiB / " + std::to_string(total) + " MiB";
}

std::string getRootStorage() {
    struct statvfs stat;

    if(statvfs("/", &stat) != 0) {
        return "Unknown";
    }

    unsigned long total = (stat.f_blocks * stat.f_frsize) / (1024 * 1024 * 1024);

    unsigned long free = (stat.f_bfree * stat.f_frsize) / (1024 * 1024 * 1024);

    unsigned long used = total - free;

    return std::to_string(used) + " GiB / " + std::to_string(total) + " GiB";
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

std::string getGPU() {
    std::string output = exec("lspci | grep -E 'VGA|3D|Display'");

    if(output.empty()) {
        return "Unknown GPU";
    }

    std::stringstream ss(output);

    std::string line;

    std::vector<std::string> gpus;

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

        if(line.find("Intel Corporation") == 0) {
            line.replace(0, 18, "Intel ");
        }

        if(line.find("NVIDIA Corporation") == 0) {
            line.replace(0, 20, "NVIDIA ");
        }

        if(line.find("Advanced Micro Devices") == 0) {
            line.replace(0, 22, "AMD ");
        }

        gpus.push_back(line);
    }

    std::string result;

    for(size_t i = 0; i < gpus.size(); i++) {
        result += gpus[i];

        if(i + 1 < gpus.size()) {
            result += " + ";
        }
    }

    return result;
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