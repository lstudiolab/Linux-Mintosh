#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>

extern "C" void mintosh_cpuid(std::uint32_t, std::uint32_t, std::uint32_t[4]);

struct Platform {
    std::string vendor, product, version, board, bios, cpu_vendor;
    bool hp, probook, hp_wmi, hp_accel;
};

static std::string dmi(const char *field) {
    std::ifstream file(std::string("/sys/class/dmi/id/") + field);
    std::string value;
    if (!file || !std::getline(file, value)) return {};
    return value;
}
static std::string cpu_vendor() {
    std::array<std::uint32_t, 4> r{};
    mintosh_cpuid(0, 0, r.data());
    char value[13]{};
    std::memcpy(value, &r[1], 4);
    std::memcpy(value + 4, &r[3], 4);
    std::memcpy(value + 8, &r[2], 4);
    return value;
}
static bool module(const char *name) {
    return access((std::string("/sys/module/") + name).c_str(), F_OK) == 0;
}
static Platform discover() {
    Platform p{dmi("sys_vendor"), dmi("product_name"), dmi("product_version"),
               dmi("board_name"), dmi("bios_version"), cpu_vendor(),
               false, false, false, false};
    p.hp = p.vendor == "HP" || p.vendor == "Hewlett-Packard" || p.vendor == "HP Inc.";
    p.probook = p.hp && p.product.find("ProBook") != std::string::npos;
    p.hp_wmi = module("hp_wmi");
    p.hp_accel = module("hp_accel");
    return p;
}
static void json_string(const std::string &s) {
    std::cout << '"';
    for (const unsigned char c : s) {
        if (c == '"' || c == '\\') std::cout << '\\' << static_cast<char>(c);
        else if (c < 0x20) std::printf("\\u%04x", c);
        else std::cout << static_cast<char>(c);
    }
    std::cout << '"';
}
int main(int argc, char **argv) {
    if (argc == 2 && std::strcmp(argv[1], "--version") == 0) {
        std::cout << "HP ProBook Compatibility 0.1.0\n"; return 0;
    }
    if (argc == 2 && std::strcmp(argv[1], "--help") == 0) {
        std::cout << "Usage: hp-compat [--json|--version|--help]\n"; return 0;
    }
    const bool json = argc == 2 && std::strcmp(argv[1], "--json") == 0;
    if (argc > 1 && !json) {
        std::cerr << "Usage: hp-compat [--json|--version|--help]\n"; return 64;
    }
    const Platform p = discover();
    if (json) {
        std::cout << "{\"vendor\":"; json_string(p.vendor);
        std::cout << ",\"product\":"; json_string(p.product);
        std::cout << ",\"product_version\":"; json_string(p.version);
        std::cout << ",\"board\":"; json_string(p.board);
        std::cout << ",\"bios_version\":"; json_string(p.bios);
        std::cout << ",\"cpu_vendor\":"; json_string(p.cpu_vendor);
        std::printf(",\"is_hp\":%s,\"is_probook\":%s,\"hp_wmi_loaded\":%s,\"hp_accel_loaded\":%s}\n",
                    p.hp ? "true" : "false", p.probook ? "true" : "false",
                    p.hp_wmi ? "true" : "false", p.hp_accel ? "true" : "false");
        return 0;
    }
    std::cout << "Vendor: " << (p.vendor.empty() ? "Unavailable" : p.vendor)
              << "\nModel: " << (p.product.empty() ? "Unavailable" : p.product)
              << "\nProduct version: " << (p.version.empty() ? "Unavailable" : p.version)
              << "\nBoard: " << (p.board.empty() ? "Unavailable" : p.board)
              << "\nBIOS: " << (p.bios.empty() ? "Unavailable" : p.bios)
              << "\nCPU vendor: " << (p.cpu_vendor.empty() ? "Unavailable" : p.cpu_vendor) << '\n';
    if (p.probook) std::cout << "Platform: HP ProBook detected. Generic x86-64 support remains active.\n";
    else if (p.hp) std::cout << "Platform: HP system detected. Generic x86-64 support remains active.\n";
    else std::cout << "Platform: Non-HP system detected or vendor unavailable. Generic x86-64 support remains active.\n";
    std::cout << "HP WMI driver loaded: " << (p.hp_wmi ? "yes" : "no")
              << "\nHP accelerometer driver loaded: " << (p.hp_accel ? "yes" : "no") << '\n'
              << "This report identifies hardware and loaded modules; it does not certify every device or firmware feature.\n";
    return 0;
}
