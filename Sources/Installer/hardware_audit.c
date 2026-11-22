#define _GNU_SOURCE
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/statvfs.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <unistd.h>

#define EXT4_SUPER_MAGIC 0xEF53
enum probe_kind { P_EXISTS, P_DIR, P_CONTAINS, P_CPUFLAG, P_MEMMIN, P_PCICLASS, P_PCIDRIVER, P_PCIATTR, P_COMMAND, P_ARCH, P_ROOT, P_WORKSPACE };
struct probe { const char *id,*name,*group; enum probe_kind kind; const char *path,*value; unsigned long long minimum; int required; };
static const struct probe probes[] = {
    {"001","uefi-firmware","Firmware and platform",P_DIR,"/sys/firmware/efi","",0ULL,1},
    {"002","efi-variables","Firmware and platform",P_DIR,"/sys/firmware/efi/efivars","",0ULL,0},
    {"003","acpi-tables","Firmware and platform",P_DIR,"/sys/firmware/acpi/tables","",0ULL,1},
    {"004","dmi-product-name","Firmware and platform",P_EXISTS,"/sys/class/dmi/id/product_name","",0ULL,0},
    {"005","dmi-system-vendor","Firmware and platform",P_EXISTS,"/sys/class/dmi/id/sys_vendor","",0ULL,0},
    {"006","dmi-board-name","Firmware and platform",P_EXISTS,"/sys/class/dmi/id/board_name","",0ULL,0},
    {"007","dmi-bios-vendor","Firmware and platform",P_EXISTS,"/sys/class/dmi/id/bios_vendor","",0ULL,0},
    {"008","efi-runtime-services","Firmware and platform",P_EXISTS,"/sys/firmware/efi/runtime","",0ULL,0},
    {"009","efi-config-tables","Firmware and platform",P_EXISTS,"/sys/firmware/efi/config_table","",0ULL,0},
    {"010","secure-boot-variable","Firmware and platform",P_EXISTS,"/sys/firmware/efi/efivars/SecureBoot-8be4df61-93ca-11d2-aa0d-00e098032b8c","",0ULL,0},
    {"011","setup-mode-variable","Firmware and platform",P_EXISTS,"/sys/firmware/efi/efivars/SetupMode-8be4df61-93ca-11d2-aa0d-00e098032b8c","",0ULL,0},
    {"012","dmi-chassis-type","Firmware and platform",P_EXISTS,"/sys/class/dmi/id/chassis_type","",0ULL,0},
    {"013","acpi-fadt","Firmware and platform",P_EXISTS,"/sys/firmware/acpi/tables/FACP","",0ULL,0},
    {"014","acpi-dsdt","Firmware and platform",P_EXISTS,"/sys/firmware/acpi/tables/DSDT","",0ULL,0},
    {"015","acpi-ssdt","Firmware and platform",P_DIR,"/sys/firmware/acpi/tables","",0ULL,0},
    {"016","x86-64-architecture","Processor",P_ARCH,"x86_64","",0ULL,1},
    {"017","cpu-long-mode","Processor",P_CPUFLAG,"/proc/cpuinfo","lm",0ULL,1},
    {"018","cpu-nx","Processor",P_CPUFLAG,"/proc/cpuinfo","nx",0ULL,1},
    {"019","cpu-sse2","Processor",P_CPUFLAG,"/proc/cpuinfo","sse2",0ULL,1},
    {"020","cpu-ssse3","Processor",P_CPUFLAG,"/proc/cpuinfo","ssse3",0ULL,0},
    {"021","cpu-sse4-1","Processor",P_CPUFLAG,"/proc/cpuinfo","sse4_1",0ULL,0},
    {"022","cpu-sse4-2","Processor",P_CPUFLAG,"/proc/cpuinfo","sse4_2",0ULL,0},
    {"023","cpu-avx","Processor",P_CPUFLAG,"/proc/cpuinfo","avx",0ULL,0},
    {"024","cpu-avx2","Processor",P_CPUFLAG,"/proc/cpuinfo","avx2",0ULL,0},
    {"025","cpu-aes","Processor",P_CPUFLAG,"/proc/cpuinfo","aes",0ULL,0},
    {"026","cpu-pclmulqdq","Processor",P_CPUFLAG,"/proc/cpuinfo","pclmulqdq",0ULL,0},
    {"027","cpu-xsave","Processor",P_CPUFLAG,"/proc/cpuinfo","xsave",0ULL,0},
    {"028","cpu-vmx","Processor",P_CPUFLAG,"/proc/cpuinfo","vmx",0ULL,0},
    {"029","cpu-svm","Processor",P_CPUFLAG,"/proc/cpuinfo","svm",0ULL,0},
    {"030","cpu-rdrand","Processor",P_CPUFLAG,"/proc/cpuinfo","rdrand",0ULL,0},
    {"031","proc-meminfo","Memory",P_EXISTS,"/proc/meminfo","",0ULL,1},
    {"032","minimum-memory-4gib","Memory",P_MEMMIN,"/proc/meminfo","MemTotal",4194304ULL,1},
    {"033","available-memory-512mib","Memory",P_MEMMIN,"/proc/meminfo","MemAvailable",524288ULL,1},
    {"034","swap-information","Memory",P_EXISTS,"/proc/swaps","",0ULL,0},
    {"035","hugepage-information","Memory",P_EXISTS,"/proc/sys/vm/nr_hugepages","",0ULL,0},
    {"036","transparent-hugepages","Memory",P_EXISTS,"/sys/kernel/mm/transparent_hugepage","",0ULL,0},
    {"037","numa-node-information","Memory",P_DIR,"/sys/devices/system/node","",0ULL,0},
    {"038","node-zero-memory","Memory",P_EXISTS,"/sys/devices/system/node/node0/meminfo","",0ULL,0},
    {"039","memory-hotplug","Memory",P_EXISTS,"/sys/devices/system/memory","",0ULL,0},
    {"040","slab-information","Memory",P_EXISTS,"/proc/slabinfo","",0ULL,0},
    {"041","buddy-information","Memory",P_EXISTS,"/proc/buddyinfo","",0ULL,0},
    {"042","zone-information","Memory",P_EXISTS,"/proc/zoneinfo","",0ULL,0},
    {"043","vmalloc-information","Memory",P_EXISTS,"/proc/vmallocinfo","",0ULL,0},
    {"044","memory-cgroup","Memory",P_EXISTS,"/sys/fs/cgroup/memory.max","",0ULL,0},
    {"045","kernel-page-size","Memory",P_EXISTS,"/proc/meminfo","",0ULL,0},
    {"046","block-device-inventory","Storage",P_DIR,"/sys/block","",0ULL,1},
    {"047","block-class-inventory","Storage",P_DIR,"/sys/class/block","",0ULL,1},
    {"048","pci-storage-controller","Storage",P_PCICLASS,"/sys/bus/pci/devices","01",0ULL,1},
    {"049","storage-controller-driver","Storage",P_PCIDRIVER,"/sys/bus/pci/devices","01",0ULL,1},
    {"050","disk-identifiers","Storage",P_DIR,"/dev/disk/by-id","",0ULL,0},
    {"051","filesystem-uuids","Storage",P_DIR,"/dev/disk/by-uuid","",0ULL,0},
    {"052","nvme-controller","Storage",P_PCICLASS,"/sys/bus/pci/devices","0108",0ULL,0},
    {"053","sata-ahci-controller","Storage",P_PCICLASS,"/sys/bus/pci/devices","0106",0ULL,0},
    {"054","virtio-block-controller","Storage",P_PCICLASS,"/sys/bus/pci/devices","0100",0ULL,0},
    {"055","scsi-host-inventory","Storage",P_DIR,"/sys/class/scsi_host","",0ULL,0},
    {"056","device-mapper-inventory","Storage",P_DIR,"/sys/class/block","dm-",0ULL,0},
    {"057","mapper-directory","Storage",P_DIR,"/dev/mapper","",0ULL,0},
    {"058","udev-utility","Storage",P_COMMAND,"udevadm","",0ULL,1},
    {"059","lsblk-utility","Storage",P_COMMAND,"lsblk","",0ULL,1},
    {"060","partition-editor","Storage",P_COMMAND,"parted","",0ULL,1},
    {"061","pci-device-bus","PCI and chipset",P_DIR,"/sys/bus/pci/devices","",0ULL,1},
    {"062","pci-vendor-identifiers","PCI and chipset",P_PCIATTR,"vendor","",0ULL,0},
    {"063","pci-device-identifiers","PCI and chipset",P_PCIATTR,"device","",0ULL,0},
    {"064","pci-class-identifiers","PCI and chipset",P_PCIATTR,"class","",0ULL,1},
    {"065","pci-modalias-data","PCI and chipset",P_PCIATTR,"modalias","",0ULL,0},
    {"066","pci-resource-data","PCI and chipset",P_PCIATTR,"resource","",0ULL,0},
    {"067","pci-subsystem-vendor","PCI and chipset",P_PCIATTR,"subsystem_vendor","",0ULL,0},
    {"068","pci-subsystem-device","PCI and chipset",P_PCIATTR,"subsystem_device","",0ULL,0},
    {"069","pci-driver-bindings","PCI and chipset",P_PCIDRIVER,"/sys/bus/pci/devices","",0ULL,0},
    {"070","display-controller-enumeration","PCI and chipset",P_PCICLASS,"/sys/bus/pci/devices","03",0ULL,1},
    {"071","network-controller-enumeration","PCI and chipset",P_PCICLASS,"/sys/bus/pci/devices","02",0ULL,1},
    {"072","usb-controller-enumeration","PCI and chipset",P_PCICLASS,"/sys/bus/pci/devices","0c03",0ULL,1},
    {"073","pci-bridge-enumeration","PCI and chipset",P_PCICLASS,"/sys/bus/pci/devices","06",0ULL,0},
    {"074","audio-controller-enumeration","PCI and chipset",P_PCICLASS,"/sys/bus/pci/devices","04",0ULL,0},
    {"075","serial-controller-enumeration","PCI and chipset",P_PCICLASS,"/sys/bus/pci/devices","07",0ULL,0},
    {"076","display-controller-present","Graphics and display",P_PCICLASS,"/sys/bus/pci/devices","03",0ULL,1},
    {"077","display-driver-bound","Graphics and display",P_PCIDRIVER,"/sys/bus/pci/devices","03",0ULL,1},
    {"078","drm-sysfs","Graphics and display",P_DIR,"/sys/class/drm","",0ULL,0},
    {"079","drm-device-node","Graphics and display",P_DIR,"/dev/dri","",0ULL,0},
    {"080","drm-card-zero","Graphics and display",P_EXISTS,"/dev/dri/card0","",0ULL,0},
    {"081","drm-render-node","Graphics and display",P_EXISTS,"/dev/dri/renderD128","",0ULL,0},
    {"082","framebuffer-device","Graphics and display",P_EXISTS,"/sys/class/graphics/fb0","",0ULL,0},
    {"083","backlight-device","Graphics and display",P_DIR,"/sys/class/backlight","",0ULL,0},
    {"084","display-edid-data","Graphics and display",P_EXISTS,"/sys/class/drm/card0-eDP-1/edid","",0ULL,0},
    {"085","display-connectors","Graphics and display",P_DIR,"/sys/class/drm","",0ULL,0},
    {"086","intel-i915-module","Graphics and display",P_EXISTS,"/sys/module/i915","",0ULL,0},
    {"087","amd-amdgpu-module","Graphics and display",P_EXISTS,"/sys/module/amdgpu","",0ULL,0},
    {"088","nouveau-module","Graphics and display",P_EXISTS,"/sys/module/nouveau","",0ULL,0},
    {"089","nvidia-module","Graphics and display",P_EXISTS,"/sys/module/nvidia","",0ULL,0},
    {"090","graphics-modeset-parameter","Graphics and display",P_EXISTS,"/sys/module/drm/parameters/modeset","",0ULL,0},
    {"091","network-controller-present","Networking",P_PCICLASS,"/sys/bus/pci/devices","02",0ULL,1},
    {"092","network-driver-bound","Networking",P_PCIDRIVER,"/sys/bus/pci/devices","02",0ULL,1},
    {"093","network-interface-inventory","Networking",P_DIR,"/sys/class/net","",0ULL,1},
    {"094","ipv4-route-table","Networking",P_EXISTS,"/proc/net/route","",0ULL,1},
    {"095","ipv6-interface-table","Networking",P_EXISTS,"/proc/net/if_inet6","",0ULL,0},
    {"096","wireless-interface-wlan0","Networking",P_EXISTS,"/sys/class/net/wlan0/wireless","",0ULL,0},
    {"097","wireless-interface-wlp2s0","Networking",P_EXISTS,"/sys/class/net/wlp2s0/wireless","",0ULL,0},
    {"098","ethernet-interface-eth0","Networking",P_EXISTS,"/sys/class/net/eth0","",0ULL,0},
    {"099","ethernet-interface-enp0s25","Networking",P_EXISTS,"/sys/class/net/enp0s25","",0ULL,0},
    {"100","network-device-class","Networking",P_DIR,"/sys/class/net","",0ULL,1},
    {"101","networkmanager-daemon","Networking",P_COMMAND,"NetworkManager","",0ULL,1},
    {"102","networkmanager-cli","Networking",P_COMMAND,"nmcli","",0ULL,1},
    {"103","wireless-supplicant","Networking",P_COMMAND,"wpa_supplicant","",0ULL,1},
    {"104","wireless-tools","Networking",P_COMMAND,"iw","",0ULL,1},
    {"105","ethernet-tools","Networking",P_COMMAND,"ethtool","",0ULL,1},
    {"106","usb-device-bus","USB and peripherals",P_DIR,"/sys/bus/usb/devices","",0ULL,1},
    {"107","usb-device-inventory","USB and peripherals",P_DIR,"/sys/bus/usb/devices","",0ULL,1},
    {"108","usb-host-controller","USB and peripherals",P_PCICLASS,"/sys/bus/pci/devices","0c03",0ULL,1},
    {"109","input-device-class","USB and peripherals",P_DIR,"/sys/class/input","",0ULL,1},
    {"110","input-device-node","USB and peripherals",P_DIR,"/dev/input","",0ULL,1},
    {"111","input-event-zero","USB and peripherals",P_EXISTS,"/dev/input/event0","",0ULL,0},
    {"112","input-device-details","USB and peripherals",P_EXISTS,"/proc/bus/input/devices","",0ULL,0},
    {"113","hidraw-device-class","USB and peripherals",P_DIR,"/sys/class/hidraw","",0ULL,0},
    {"114","hid-device-bus","USB and peripherals",P_DIR,"/sys/bus/hid/devices","",0ULL,0},
    {"115","tty-device-class","USB and peripherals",P_DIR,"/sys/class/tty","",0ULL,0},
    {"116","serial-console-device","USB and peripherals",P_EXISTS,"/dev/ttyS0","",0ULL,0},
    {"117","bluetooth-device-class","USB and peripherals",P_DIR,"/sys/class/bluetooth","",0ULL,0},
    {"118","sound-device-class","USB and peripherals",P_DIR,"/sys/class/sound","",0ULL,0},
    {"119","alsa-card-inventory","USB and peripherals",P_EXISTS,"/proc/asound/cards","",0ULL,0},
    {"120","audio-device-node","USB and peripherals",P_DIR,"/dev/snd","",0ULL,0},
    {"121","installer-runs-as-root","Security capabilities",P_ROOT,"","",0ULL,1},
    {"122","kernel-lockdown-interface","Security capabilities",P_EXISTS,"/sys/kernel/security/lockdown","",0ULL,0},
    {"123","secure-boot-state-interface","Security capabilities",P_EXISTS,"/sys/firmware/efi/efivars/SecureBoot-8be4df61-93ca-11d2-aa0d-00e098032b8c","",0ULL,0},
    {"124","tpm-device","Security capabilities",P_EXISTS,"/dev/tpm0","",0ULL,0},
    {"125","tpm-resource-manager","Security capabilities",P_EXISTS,"/dev/tpmrm0","",0ULL,0},
    {"126","tpm-class","Security capabilities",P_DIR,"/sys/class/tpm","",0ULL,0},
    {"127","ima-security-interface","Security capabilities",P_DIR,"/sys/kernel/security/ima","",0ULL,0},
    {"128","apparmor-security-interface","Security capabilities",P_EXISTS,"/sys/module/apparmor","",0ULL,0},
    {"129","selinux-security-interface","Security capabilities",P_EXISTS,"/sys/fs/selinux","",0ULL,0},
    {"130","seccomp-actions-interface","Security capabilities",P_EXISTS,"/proc/sys/kernel/seccomp/actions_avail","",0ULL,0},
    {"131","kernel-aslr-control","Security capabilities",P_EXISTS,"/proc/sys/kernel/randomize_va_space","",0ULL,0},
    {"132","module-disable-control","Security capabilities",P_EXISTS,"/proc/sys/kernel/modules_disabled","",0ULL,0},
    {"133","device-mapper-crypt-module","Security capabilities",P_EXISTS,"/sys/module/dm_crypt","",0ULL,0},
    {"134","cryptsetup-utility","Security capabilities",P_COMMAND,"cryptsetup","",0ULL,0},
    {"135","efi-variable-filesystem","Security capabilities",P_EXISTS,"/sys/firmware/efi/efivars","",0ULL,0},
    {"136","git-client","Build, network and workspace",P_COMMAND,"git","",0ULL,1},
    {"137","c-compiler","Build, network and workspace",P_COMMAND,"gcc","",0ULL,1},
    {"138","cxx-compiler","Build, network and workspace",P_COMMAND,"g++","",0ULL,1},
    {"139","make-builder","Build, network and workspace",P_COMMAND,"make","",0ULL,1},
    {"140","debootstrap-builder","Build, network and workspace",P_COMMAND,"debootstrap","",0ULL,1},
    {"141","curl-client","Build, network and workspace",P_COMMAND,"curl","",0ULL,1},
    {"142","python3-validator","Build, network and workspace",P_COMMAND,"python3","",0ULL,1},
    {"143","bc-kernel-tool","Build, network and workspace",P_COMMAND,"bc","",0ULL,1},
    {"144","bison-kernel-tool","Build, network and workspace",P_COMMAND,"bison","",0ULL,1},
    {"145","flex-kernel-tool","Build, network and workspace",P_COMMAND,"flex","",0ULL,1},
    {"146","libelf-development-header","Build, network and workspace",P_EXISTS,"/usr/include/libelf.h","",0ULL,1},
    {"147","ncurses-development-header","Build, network and workspace",P_EXISTS,"/usr/include/ncurses.h","",0ULL,1},
    {"148","gnu-efi-development-header","Build, network and workspace",P_EXISTS,"/usr/include/efi/efi.h","",0ULL,1},
    {"149","qemu-system-x86","Build, network and workspace",P_COMMAND,"qemu-system-x86_64","",0ULL,1},
    {"150","persistent-build-workspace","Build, network and workspace",P_WORKSPACE,"","40GiB-ext4",0ULL,1},
};
static int read_text(const char *path, char *buffer, size_t size) {
    FILE *f=fopen(path,"r"); if(!f) return -1;
    size_t n=fread(buffer,1,size-1,f); fclose(f); buffer[n]='\\0'; return 0;
}
static int directory_nonempty(const char *path) {
    DIR *d=opendir(path); if(!d) return 0; struct dirent *e; int found=0;
    while((e=readdir(d))) if(strcmp(e->d_name,".") && strcmp(e->d_name,"..")) { found=1; break; }
    closedir(d); return found;
}
static int pci_match(const char *prefix,int require_driver) {
    DIR *d=opendir("/sys/bus/pci/devices"); if(!d) return 0;
    struct dirent *e; int found=0;
    while((e=readdir(d))) {
        if(e->d_name[0]=='.') continue;
        char path[PATH_MAX],value[128];
        snprintf(path,sizeof(path),"/sys/bus/pci/devices/%s/class",e->d_name);
        if(read_text(path,value,sizeof(value))!=0) continue;
        char *p=value; while(*p==' '||*p=='\t') p++;
        if(strncmp(p,"0x",2)==0) p+=2;
        if(strncmp(p,prefix,strlen(prefix))!=0) continue;
        if(require_driver) {
            snprintf(path,sizeof(path),"/sys/bus/pci/devices/%s/driver",e->d_name);
            if(access(path,F_OK)!=0) continue;
        }
        found=1; break;
    }
    closedir(d); return found;
}
static int command_exists(const char *name) {
    if(strchr(name,'/')) return access(name,X_OK)==0;
    const char *env=getenv("PATH"); if(!env) env="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin";
    char *copy=strdup(env); if(!copy) return 0;
    int found=0; char *save=NULL;
    for(char *part=strtok_r(copy,":",&save);part;part=strtok_r(NULL,":",&save)) {
        char candidate[PATH_MAX]; snprintf(candidate,sizeof(candidate),"%s/%s",*part?part:".",name);
        if(access(candidate,X_OK)==0) { found=1; break; }
    }
    free(copy); return found;
}
static int contains(const char *path,const char *needle) {
    char buffer[1024*1024]; if(read_text(path,buffer,sizeof(buffer))!=0) return 0;
    return strstr(buffer,needle)!=NULL;
}
static int memory_at_least(const char *key,unsigned long long minimum) {
    char buffer[65536]; if(read_text("/proc/meminfo",buffer,sizeof(buffer))!=0) return 0;
    char *line=strstr(buffer,key); if(!line) return 0;
    unsigned long long value=0; if(sscanf(line+strlen(key),": %llu",&value)!=1) return 0;
    return value>=minimum;
}
static int workspace_ok(const char *path) {
    struct statfs fs; struct statvfs v;
    if(!path || !*path || statfs(path,&fs)!=0 || statvfs(path,&v)!=0) return 0;
    if((unsigned long)fs.f_type!=EXT4_SUPER_MAGIC) return 0;
    unsigned long long free_bytes=(unsigned long long)v.f_bavail*(unsigned long long)v.f_frsize;
    return free_bytes>=40ULL*1024ULL*1024ULL*1024ULL;
}
static int run_probe(const struct probe *p,const char *workspace) {
    struct stat st; char buffer[4096];
    switch(p->kind) {
        case P_EXISTS: return stat(p->path,&st)==0;
        case P_DIR: return directory_nonempty(p->path);
        case P_CONTAINS: return contains(p->path,p->value);
        case P_CPUFLAG: {
            if(!contains(p->path,p->value)) return 0;
            snprintf(buffer,sizeof(buffer)," %s ",p->value);
            return contains(p->path,buffer);
        }
        case P_MEMMIN: return memory_at_least(p->value,p->minimum);
        case P_PCICLASS: return pci_match(p->value,0);
        case P_PCIDRIVER: return pci_match(p->value,1);
        case P_PCIATTR: {
            DIR *d=opendir("/sys/bus/pci/devices"); if(!d) return 0;
            struct dirent *e; int found=0;
            while((e=readdir(d))) { if(e->d_name[0]=='.') continue;
                snprintf(buffer,sizeof(buffer),"/sys/bus/pci/devices/%s/%s",e->d_name,p->path);
                if(access(buffer,F_OK)==0) { found=1; break; }
            }
            closedir(d); return found;
        }
        case P_COMMAND: return command_exists(p->path);
        case P_ARCH: { struct utsname u; return uname(&u)==0 && strcmp(u.machine,p->path)==0; }
        case P_ROOT: return geteuid()==0;
        case P_WORKSPACE: return workspace_ok(workspace);
    }
    return 0;
}
int main(int argc,char **argv) {
    const char *workspace=NULL; int list=0,self=0;
    for(int i=1;i<argc;i++) {
        if(!strcmp(argv[i],"--list")) list=1;
        else if(!strcmp(argv[i],"--self-test")) self=1;
        else if(!strcmp(argv[i],"--workspace") && i+1<argc) workspace=argv[++i];
        else { fprintf(stderr,"usage: %s [--list|--self-test] [--workspace PATH]\\n",argv[0]); return 64; }
    }
    const size_t count=sizeof(probes)/sizeof(probes[0]);
    if(list || self) {
        for(size_t i=0;i<count;i++) printf("%s\\t%s\\t%s\\n",probes[i].id,probes[i].group,probes[i].name);
        if(self) {
            if(count!=150) { fprintf(stderr,"expected 150 checks, found %zu\\n",count); return 1; }
            for(size_t i=0;i<count;i++) for(size_t j=i+1;j<count;j++)
                if(!strcmp(probes[i].id,probes[j].id)||!strcmp(probes[i].name,probes[j].name)) {
                    fprintf(stderr,"duplicate hardware check: %s\\n",probes[i].id); return 1;
                }
            fprintf(stderr,"PASS: exactly 150 unique hardware compatibility checks are registered.\\n");
        }
        return 0;
    }
    if(!workspace) { fprintf(stderr,"hardware audit requires --workspace PATH\\n"); return 64; }
    unsigned pass=0,skip=0,fail=0;
    printf("Mintosh hardware compatibility audit: %zu checks\\n",count);
    for(size_t i=0;i<count;i++) {
        const struct probe *p=&probes[i]; int ok=run_probe(p,workspace);
        if(ok) { pass++; printf("PASS %s %s\\n",p->id,p->name); }
        else if(!p->required) { skip++; printf("SKIP %s %s (optional capability not present)\\n",p->id,p->name); }
        else { fail++; printf("FAIL %s %s (required capability missing)\\n",p->id,p->name); }
    }
    printf("SUMMARY total=%zu pass=%u skip=%u fail=%u\\n",count,pass,skip,fail);
    return fail?1:0;
}
