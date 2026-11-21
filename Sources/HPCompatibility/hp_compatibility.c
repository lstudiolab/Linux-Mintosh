#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

static void read_dmi(const char *name, char *value, size_t capacity) {
    char path[256];
    snprintf(path, sizeof path, "/sys/class/dmi/id/%s", name);
    FILE *file = fopen(path, "r");
    if (!file) { value[0] = '\0'; return; }
    if (!fgets(value, (int)capacity, file)) value[0] = '\0';
    fclose(file);
    value[strcspn(value, "\r\n")] = '\0';
}
static int hp_vendor(const char *vendor) {
    return strcasecmp(vendor, "HP") == 0 ||
           strcasecmp(vendor, "Hewlett-Packard") == 0 ||
           strcasecmp(vendor, "HP Inc.") == 0;
}
static void print_json_string(const char *value) {
    putchar('"');
    for (const unsigned char *p = (const unsigned char *)value; *p; ++p) {
        if (*p == '"' || *p == '\\') { putchar('\\'); putchar(*p); }
        else if (*p < 0x20) printf("\\u%04x", *p);
        else putchar(*p);
    }
    putchar('"');
}
static int module_available(const char *name) {
    char path[128];
    snprintf(path, sizeof path, "/sys/module/%s", name);
    return access(path, F_OK) == 0;
}
int main(int argc, char **argv) {
    if (argc == 2 && strcmp(argv[1], "--version") == 0) {
        puts("HP ProBook Compatibility 0.1.0"); return 0;
    }
    int json = argc == 2 && strcmp(argv[1], "--json") == 0;
    if (argc == 2 && strcmp(argv[1], "--help") == 0) {
        puts("Usage: hp-compat [--json|--version|--help]"); return 0;
    }
    if (argc > 1 && !json) {
        fputs("Usage: hp-compat [--json|--version|--help]\n", stderr); return 64;
    }
    char vendor[128], product[256], version[128], board[128], bios[128];
    read_dmi("sys_vendor", vendor, sizeof vendor);
    read_dmi("product_name", product, sizeof product);
    read_dmi("product_version", version, sizeof version);
    read_dmi("board_name", board, sizeof board);
    read_dmi("bios_version", bios, sizeof bios);
    int hp = hp_vendor(vendor);
    int probook = hp && strstr(product, "ProBook") != NULL;
    int hp_wmi = module_available("hp_wmi");
    int hp_accel = module_available("hp_accel");
    if (json) {
        fputs("{\"vendor\":", stdout); print_json_string(vendor);
        fputs(",\"product\":", stdout); print_json_string(product);
        fputs(",\"product_version\":", stdout); print_json_string(version);
        fputs(",\"board\":", stdout); print_json_string(board);
        fputs(",\"bios_version\":", stdout); print_json_string(bios);
        printf(",\"is_hp\":%s,\"is_probook\":%s,\"hp_wmi_loaded\":%s,\"hp_accel_loaded\":%s}\n",
               hp ? "true" : "false", probook ? "true" : "false",
               hp_wmi ? "true" : "false", hp_accel ? "true" : "false");
        return 0;
    }
    printf("Vendor: %s\nModel: %s\nProduct version: %s\nBoard: %s\nBIOS: %s\n",
           vendor[0] ? vendor : "Unavailable", product[0] ? product : "Unavailable",
           version[0] ? version : "Unavailable", board[0] ? board : "Unavailable",
           bios[0] ? bios : "Unavailable");
    if (probook) puts("Platform: HP ProBook detected. Mintosh HP laptop driver and firmware support is enabled.");
    else if (hp) puts("Platform: HP system detected. Generic HP laptop support is enabled; this model is not identified as a ProBook.");
    else puts("Platform: HP system not detected. Generic Linux laptop support remains enabled.");
    printf("HP WMI driver loaded: %s\nHP accelerometer driver loaded: %s\n",
           hp_wmi ? "yes" : "no", hp_accel ? "yes" : "no");
    puts("This report identifies hardware and loaded modules; it does not certify every device or firmware feature.");
    return 0;
}
