#include "system.hpp"

#include <X11/extensions/randr.h>
#include <cctype>
#include <cstddef>
#include <iostream>
#include <iomanip>
#include <ios>
#include <string_view>
#include <sys/types.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/statvfs.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include <linux/fb.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "../include/nlohmann/json.hpp"

#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <array>
#include <cstdio>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <filesystem>
#include <optional>
#include <cstring>

using json = nlohmann::json;

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

    std::string unit = "GiB";

    if(gib < 1.0) {
        gib *= 1024.0;

        unit = "MiB";

        if(gib < 1.0) {
            gib *= 1024;

            unit = "KiB";
        }
    }

    ss << std::fixed << std::setprecision(1) << gib << unit;

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

std::string readFile(const std::filesystem::path& path) {
    std::ifstream file(path);

    if(!file.is_open()) {
        return "";
    }

    std::string value;
    std::getline(file, value);

    return value;
}

std::filesystem::path getPCIIDsPath() {
    const std::vector<std::filesystem::path> paths = {
        "/usr/share/hwdata/pci.ids",
        "/usr/share/misc/pci.ids"
    };

    for(const auto& path : paths) {
        if(std::filesystem::exists(path)) {
            return path;
        }
    }

    return "";
}

struct PCIInfo {
    std::string vendor;
    std::string device;
};

std::optional<PCIInfo> lookupPCIName(
    const std::string& vendorID,
    const std::string& deviceID
) {
    auto pciPath = getPCIIDsPath();

    if(pciPath.empty()) {
        return std::nullopt;
    }

    std::ifstream file(pciPath);

    if(!file.is_open()) {
        return std::nullopt;
    }

    const std::string vendorHex = vendorID.substr(2);
    const std::string deviceHex = deviceID.substr(2);

    std::string line;

    bool insideVendor = false;
    std::string vendorName;

    while(std::getline(file, line)) {
        if(line.empty()) {
            continue;
        }

        if(line.size() >= 4 && std::isxdigit(line[0]) && std::isxdigit(line[1]) && std::isxdigit(line[2]) && std::isxdigit(line[3])) {
            std::string currentVendor = line.substr(0, 4);

            insideVendor = (currentVendor == vendorHex);

            if(insideVendor && line.size() > 6) {
                vendorName = line.substr(6);
            }

            continue;
        }

        if(!insideVendor) {
            continue;
        }

        if(line.size() >= 2 && std::isspace(line[0]) && std::isspace(line[1])) {
            continue;
        }

        size_t start = line.find_first_not_of(" \t");

        if(start == std::string::npos) {
            continue;
        }

        if(line.size() < start + 4) {
            continue;
        }

        std::string currentDevice = line.substr(start, 4);

        if(currentDevice == deviceHex) {
            std::string deviceName;

            if(line.size() > start + 6) {
                deviceName = line.substr(start + 6);
            }

            return PCIInfo {
                vendorName,
                deviceName
            };
        }
    }

    return std::nullopt;
}

std::string cleanGPUName(
    const std::string& vendor,
    std::string device
) {
    size_t bracketStart = device.find('[');
    size_t bracketEnd = device.find(']');

    if(bracketStart != std::string::npos && bracketEnd != std::string::npos && bracketEnd > bracketStart) {
        device = device.substr(bracketStart + 1, bracketEnd - bracketStart - 1);
    }

    const std::vector<std::string> removePrefixes = {
        "Corporation",
        "Inc.",
        "Advanced Micro Devices"
    };

    std::string cleanVendor = vendor;

    for(const auto& prefix : removePrefixes) {
        size_t pos = cleanVendor.find(prefix);

        if(pos != std::string::npos) {
            cleanVendor.erase(pos, prefix.length());
        }
    }

    while(!cleanVendor.empty() && cleanVendor.back() == ' ') {
        cleanVendor.pop_back();
    }

    if(cleanVendor.find("AMD") != std::string::npos || cleanVendor.find("Advanced Micro Devices") != std::string::npos) {
    cleanVendor = "AMD";
    } else if(cleanVendor.find("NVIDIA") != std::string::npos) {
        cleanVendor = "Nvidia";
    } else if(cleanVendor.find("Intel") != std::string::npos) {
        cleanVendor = "Intel";
    }

    return cleanVendor + " " + device;
}

std::vector<std::string> getGPU() {
    std::vector<std::string> gpus;
    
    const std::filesystem::path drmPath = "/sys/class/drm";

    if(!std::filesystem::exists(drmPath)) {
        return { "Unknown GPU" };
    }

    for(const auto& entry : std::filesystem::directory_iterator(drmPath)) {
        std::string cardName = entry.path().filename().string();

        if(cardName.find("card") != 0 || cardName.find('-') != std::string::npos) {
            continue;
        }

        auto devicePath = entry.path() / "device";

        std::string vendor = readFile(devicePath / "vendor");

        std::string device = readFile(devicePath / "device");

        if(vendor.empty() || device.empty()) {
            continue;
        }

        auto gpuName = lookupPCIName(vendor, device);

        if(gpuName.has_value()) {
            gpus.push_back(
                cleanGPUName(
                    gpuName->vendor,
                    gpuName->device
                )
            );
        } else {
            gpus.push_back(
                "Unknown GPU (" + device + ")"
            );
        }
    }

    if(gpus.empty()) {
        return { "Unknown GPU" };
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

pid_t getParentPID(pid_t pid) {
    std::ifstream file(
        "/proc/" + std::to_string(pid) + "/stat"
    );

    if(!file.is_open()) {
        return -1;
    }

    std::string tmp;
    pid_t ppid;

    file >> tmp >> tmp >> tmp >> ppid;

    return ppid;
}

std::string getProcessName(pid_t pid) {
    std::ifstream file(
        "/proc/" + std::to_string(pid) + "/comm"
    );

    if(!file.is_open()) {
        return "";
    }

    std::string name;

    std::getline(file, name);

    return name;
}

std::string prettifyTerminal(
    const std::string& terminal
) {
    if(terminal == "kitty") {
        return "Kitty";
    }

    if(terminal == "konsole") {
        return "Konsole";
    }

    if(terminal == "kgx") {
        return "GNOME Console";
    }

    if(terminal == "gnome-terminal-") {
        return "GNOME Terminal";
    }

    if(terminal == "alacritty") {
        return "Alacritty";
    }

    if(terminal == "wezterm") {
        return "WezTerm";
    }

    if(terminal == "foot") {
        return "Foot";
    }

    if(terminal == "ghostty") {
        return "Ghostty";
    }

    if(terminal == "st") {
        return "st";
    }

    if(terminal == "urxvt") {
        return "URxvt";
    }

    if(terminal == "tilix") {
        return "Tilix";
    }

    return terminal;
}

std::string getTerminal() {
    static const std::vector<std::string> terminals = {
        "kitty",
        "konsole",
        "kgx",
        "gnome-terminal-",
        "alacritty",
        "wezterm",
        "foot",
        "ghostty",
        "xterm",
        "st",
        "urxvt",
        "terminator",
        "tilix"
    };

    pid_t pid = getpid();

    while(pid > 1) {
        std::string process = getProcessName(pid);

        for(const auto& term : terminals) {
            if(process == term) {
                return prettifyTerminal(term);
            }
        }

        pid = getParentPID(pid);
    }

    const char* termProgram = getenv("TERM_PROGRAM");

    if(termProgram) {
        return termProgram;
    }

    const char* term = getenv("TERM");

    if(term) {
        return term;
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

bool commandExists(std::string_view cmd) {
    const char* pathEnv = getenv("PATH");

    if(!pathEnv) {
        return false;
    }

    std::stringstream ss(pathEnv);
    std::string path;

    while(std::getline(ss, path, ':')) {
        std::string full = path + "/" + std::string(cmd);

        if(access(full.c_str(), X_OK) == 0) {
            return true;
        }
    }

    return false;
}

size_t countDirectories(const std::filesystem::path& path) {
    if(!std::filesystem::exists(path)) {
        return 0;
    }

    size_t count = 0;

    for(const auto& entry : std::filesystem::directory_iterator(path)) {
        if(entry.is_directory()) {
            count++;
        }
    }

    return count;
}

size_t countFilesWithExtension(
    const std::filesystem::path& path,
    const std::string& extension
) {
    if(!std::filesystem::exists(path)) {
        return 0;
    }

    size_t count = 0;

    for(const auto& entry : std::filesystem::directory_iterator(path)) {
        if(entry.path().extension() == extension) {
            count++;
        }
    }

    return count;
}

size_t countLines(const std::string& text) {
    if(text.empty()) {
        return 0;
    }

    return std::count(
        text.begin(),
        text.end(),
        '\n'
    );
}

std::string getPackages() {
    std::vector<std::string> packages;

    size_t pacmanCount = countDirectories("/var/lib/pacman/local");

    if(pacmanCount > 0) {
        packages.push_back(
            std::to_string(pacmanCount) + " (pacman)"
        );
    }

    size_t dpkgCount = countFilesWithExtension(
        "/var/lib/dpkg/info",
        ".list"
    );

    if(dpkgCount > 0) {
        packages.push_back(
            std::to_string(dpkgCount) + " (dpkg)"
        );
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

    size_t emergeCount = 0;

    std::filesystem::path emergePath = "/var/db/pkg";

    if(std::filesystem::exists(emergePath)) {
        for(const auto& category : std::filesystem::directory_iterator(emergePath)) {
            if(!category.is_directory()) {
                continue;
            }

            emergeCount += countDirectories(category.path());
        }
    }

    if(emergeCount > 0) {
        packages.push_back(
            std::to_string(emergeCount) + " (emerge)"
        );
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
        std::string output = exec("flatpak list --columns=application");

        size_t count = countLines(output);

        if(count > 0) {
            packages.push_back(
                std::to_string(count) + " (flatpak)"
            );
        }
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

std::string makeSeparator() {
    return std::string(getUsername().size() + getHostname().size() + 1, '-');
}

static std::string command(const char* cmd) {
    std::array<char, 256> buffer{};

    std::string result;

    FILE* pipe = popen(cmd, "r");

    if(!pipe) {
        return "";
    }

    while(fgets(buffer.data(), buffer.size(), pipe)) {
        result += buffer.data();
    }

    pclose(pipe);

    return result;
}

static bool isWayland() {
    return std::getenv("WAYLAND_DISPLAY") != nullptr;
}

static std::vector<std::string> hyprlandDisplays() {
    std::vector<std::string> modes;

    std::string output = command("hyprctl monitors -j 2>/dev/null");

    if(output.empty()) {
        return modes;
    }

    size_t pos = 0;

    while((pos = output.find("\"width\": ", pos)) != std::string::npos) {
        pos += 8;

        size_t end = output.find(',', pos);

        std::string width = output.substr(pos, end - pos);

        size_t hpos = output.find("\"height\": ", end);

        if(hpos == std::string::npos) {
            break;
        }

        hpos += 9;

        size_t hend = output.find(',', hpos);

        std::string height = output.substr(hpos, hend - hpos);

        modes.push_back(width + "x" + height);

        pos = hend;
    }

    return modes;
}


static std::vector<std::string> swayDisplays() {
    std::vector<std::string> modes;

    std::string output = command("swaymsg -t get_outputs 2>/dev/null");

    if(output.empty()) {
        return modes;
    }

    size_t pos = 0;

    while((pos = output.find("\"current_mode\"", pos)) != std::string::npos) {
        size_t wpos = output.find("\"width\":", pos);

        size_t hpos = output.find("\"height\":", pos);

        if(wpos == std::string::npos || hpos == std::string::npos) {
            break;
        }

        wpos += 8;
        hpos += 9;

        size_t wend = output.find(',', wpos);

        size_t hend = output.find(',', hpos);

        std::string width = output.substr(wpos, wend - wpos);

        std::string height = output.substr(hpos, hend - hpos);

        modes.push_back(width + "x" + height);

        pos = hend;
    }

    return modes;
}

static std::vector<std::string> niriDisplays() {
    std::vector<std::string> modes;

    std::string output = command("niri msg outputs 2>/dev/null");

    if(output.empty()) {
        return modes;
    }

    size_t pos = 0;

    while((pos = output.find("Current mode:", pos)) != std::string::npos) {
        pos += 13;

        size_t end = output.find('@', pos);

        if(end == std::string::npos) {
            break;
        }

        std::string mode = output.substr(pos, end - pos);

        while(!mode.empty() && mode.front() == ' ') {
            mode.erase(mode.begin());
        }

        while(!mode.empty() && mode.back() == ' ') {
            mode.pop_back();
        }

        size_t x = mode.find('x');

        if(x != std::string::npos) {
            std::string width = mode.substr(0, x);

            std::string height = mode.substr(x + 1);

            mode = width + "x" + height;
        }

        modes.push_back(mode);
    }

    return modes;
}

static std::vector<std::string> kdeDisplays() {
    std::vector<std::string> modes;

    std::string output = command("kscreen-doctor -j 2>/dev/null");

    if(output.empty()) {
        return modes;
    }

    try {
        json j = json::parse(output);

        if(!j.contains("outputs")) {
            return modes;
        }

        for(const auto& screen : j["outputs"]) {
            if(!screen.value("connected", false) || !screen.value("enabled", false)) {
                continue;
            }

            const auto& size = screen["size"];

            int width = size.value("width", 0);
            int height = size.value("height", 0);

            if(width > 0 && height > 0) {
                modes.push_back(
                    std::to_string(width) + "x" + std::to_string(height)
                );
            }
        }
    }
    catch(const std::exception&) {
        return modes;
    }

    return modes;
}

static std::vector<std::string> gnomeDisplays() {
    std::vector<std::string> modes;

    std::string output = command(
        "gdbus call "
        "--session "
        "--dest org.gnome.Mutter.DisplayConfig "
        "--object-path /org/gnome/Mutter/DisplayConfig "
        "--method org.gnome.Mutter.DisplayConfig.GetCurrentState "
        "2>/dev/null"
    );

    if(output.empty()) {
        return modes;
    }

    size_t pos = 0;

    while((pos = output.find("width", pos)) != std::string::npos) {
        size_t numStart = output.find_first_of("0123456789", pos);

        if(numStart == std::string::npos) {
            break;
        }

        size_t numEnd = output.find_first_not_of("0123456789", numStart);

        std::string width = output.substr(
            numStart,
            numEnd - numStart
        );

        size_t hpos = output.find("height", numEnd);

        if(hpos == std::string::npos) {
            break;
        }

        size_t hStart = output.find_first_of("0123456789", hpos);

        if(hStart == std::string::npos) {
            break;
        }

        size_t hEnd = output.find_first_not_of("0123456789", hStart);

        std::string height = output.substr(
            hStart,
            hEnd - hStart
        );

        std::string mode = width + "x" + height;

        if(std::find(
            modes.begin(),
            modes.end(),
            mode
        ) == modes.end()) {
            modes.push_back(mode);
        }

        pos = hEnd;
    }

    return modes;
}

static std::vector<std::string> waylandDisplays() {
    if(std::getenv("HYPRLAND_INSTANCE_SIGNATURE")) {
        auto hypr = hyprlandDisplays();

        if(!hypr.empty()) {
            return hypr;
        }
    }

    if(std::getenv("SWAYSOCK")) {
        auto sway = swayDisplays();

        if(!sway.empty()) {
            return sway;
        }
    }

    if(std::getenv("NIRI_SOCKET")) {
        auto niri = niriDisplays();

        if(!niri.empty()) {
            return niri;
        }
    }

    const char* desktop = std::getenv("XDG_CURRENT_DESKTOP");

    if(desktop) {
        std::string de = desktop;

        if(de.find("KDE") != std::string::npos) {
            auto kde = kdeDisplays();

            if(!kde.empty()) {
                return kde;
            }
        }

        if(de.find("GNOME") != std::string::npos) {
            auto gnome = gnomeDisplays();

            if(!gnome.empty()) {
                return gnome;
            }
        }
    }

    return {};
}

static std::string trim(std::string s) {
    while(!s.empty() && (s.back() == '\n' || s.back() == '\r' || s.back() == ' ')) {
        s.pop_back();
    }

    return s;
}

static std::string formatMode(const std::string& mode) {
    size_t x = mode.find('x');

    if(x == std::string::npos) {
        return mode;
    }

    return mode.substr(0, x) + "x" + mode.substr(x + 1);
}

static std::string joinModes(
    const std::vector<std::string>& modes
) {
    if(modes.empty()) {
        return "Unknown";
    }

    std::stringstream ss;

    for (size_t i = 0; i < modes.size(); ++i) {
        ss << formatMode(modes[i]);

        if(i + 1 < modes.size()) {
            ss << " + ";
        }
    }

    return ss.str();
}

static std::vector<std::string> drmDisplays() {
    std::vector<std::string> modes;

    const std::filesystem::path drm("/sys/class/drm");

    if(std::filesystem::exists(drm)) {
        return modes;
    }

    for(const auto& entry : std::filesystem::directory_iterator(drm)) {
        const std::string name = entry.path().filename().string();

        if(name.find('-') == std::string::npos) {
            continue;
        }

        if(name.find("Virtual") != std::string::npos || name.find("None") != std::string::npos) {
            continue;
        }

        std::ifstream statusFile(entry.path() / "status");

        if(!statusFile.good()) {
            continue;
        }

        std::string status;
        std::getline(statusFile, status);

        if(status != "connected") {
            continue;
        }

        std::ifstream modesFile(entry.path() / "modes");

        if(!modesFile.good()) {
            continue;
        }

        std::string mode;
        std::getline(modesFile, mode);

        mode = trim(mode);

        if(!mode.empty()) {
            modes.push_back(mode);
        }
    }

    return modes;
}

static std::vector<std::string> xrandrDisplays() {
    std::vector<std::string> modes;

    Display* display = XOpenDisplay(nullptr);

    if(!display) {
        return modes;
    }

    Window root = DefaultRootWindow(display);

    XRRScreenResources* screen = XRRGetScreenResourcesCurrent(display, root);

    if(!screen) {
        XCloseDisplay(display);
        return modes;
    }

    for(int i = 0; i < screen->noutput; ++i) {
        XRROutputInfo* output = XRRGetOutputInfo(display, screen, screen->outputs[i]);

        if(!output) {
            continue;
        }

        if(output->connection != RR_Connected || output->crtc == 0) {
            XRRFreeOutputInfo(output);
            continue;
        }

        XRRCrtcInfo* crtc = XRRGetCrtcInfo(
            display,
            screen,
            output->crtc
        );

        if(!crtc) {
            XRRFreeOutputInfo(output);
            continue;
        }

        std::string mode = std::to_string(crtc->width) + "x" + std::to_string(crtc->height);

        modes.push_back(mode);

        XRRFreeCrtcInfo(crtc);
        XRRFreeOutputInfo(output);
    }

    XRRFreeScreenResources(screen);
    XCloseDisplay(display);

    return modes;
}

static std::vector<std::string> framebufferDisplay() {
    std::vector<std::string> modes;

    int fb = open("/dev/fb0", O_RDONLY);

    if(fb < 0) {
        return modes;
    }

    fb_var_screeninfo info{};

    if(ioctl(fb, FBIOGET_VSCREENINFO, &info) == 0) {
        std::string mode = std::to_string(info.xres) + "x" + std::to_string(info.yres);

        modes.push_back(mode);
    }

    close(fb);

    return modes;
}

std::string getDisplay() {
    
    if(isWayland()) {
        auto wl = waylandDisplays();

        if(!wl.empty()) {
            return joinModes(wl);
        }
    }

    auto x11 = xrandrDisplays();

    if(!x11.empty()) {
        return joinModes(x11);
    }

    auto drm = drmDisplays();

    if(!drm.empty()) {
        return joinModes(drm);
    }

    auto fb = framebufferDisplay();

    if(!fb.empty()) {
        return joinModes(fb);
    }

    return "Unknown";
}

static std::string routeIP() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    if(sock < 0) {
        return "";
    }

    sockaddr_in server{};

    server.sin_family = AF_INET;
    server.sin_port = htons(80);

    inet_pton(
        AF_INET,
        "8.8.8.8",
        &server.sin_addr
    );

    if(connect(
        sock,
        (sockaddr*)&server,
        sizeof(server)
    ) < 0) {
        close(sock);
        return "";
    }

    sockaddr_in local{};
    socklen_t len = sizeof(local);

    if(getsockname(
        sock,
        (sockaddr*)&local,
        &len
    ) < 0) {
        close(sock);
        return "";
    }

    char buffer[INET_ADDRSTRLEN];

    const char* result = inet_ntop(
        AF_INET,
        &local.sin_addr,
        buffer,
        sizeof(buffer)
    );

    close(sock);

    if(!result) {
        return "";
    }

    return std::string(buffer);
}

static bool invalidInterface(const std::string& name) {
    return 
        name == "lo" ||

        name.find("docker") == 0 ||
        name.find("veth") == 0 ||
        name.find("virbr") == 0 ||
        name.find("br-") == 0 ||
        name.find("tun") == 0;
}

static std::string interfaceIP() {
    ifaddrs* ifaddr = nullptr;

    if(getifaddrs(&ifaddr) == -1) {
        return "";
    }

    std::string ip;

    for(ifaddrs* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if(ifa->ifa_addr) {
            continue;
        }

        if(ifa->ifa_addr->sa_family != AF_INET) {
            continue;
        }

        if(!(ifa->ifa_flags & IFF_UP)) {
            continue;
        }

        std::string name = ifa->ifa_name;

        if(invalidInterface(name)) {
            continue;
        }

        char host[INET_ADDRSTRLEN];

        sockaddr_in* addr = (sockaddr_in*)ifa->ifa_addr;

        if(!inet_ntop(
            AF_INET,
            &addr->sin_addr,
            host,
            sizeof(host)
        )) {
            continue;
        }

        ip = host;
        break;
    } 

    freeifaddrs(ifaddr);

    return ip;
}

std::string getLocalIP() {
    std::string ip = routeIP();

    if(!ip.empty()) {
        return ip;
    }

    ip = interfaceIP();

    if(!ip.empty()) {
        return ip;
    }

    return "Unknown";
}