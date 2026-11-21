#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/utsname.h>
static void list(const char *label,const char *path){DIR *d=opendir(path);printf("%s:\n",label);if(!d){puts("  unavailable");return;}struct dirent *e;while((e=readdir(d)))if(e->d_name[0]!='.')printf("  %s\n",e->d_name);closedir(d);}
int main(int argc,char **argv){if(argc>1&&!strcmp(argv[1],"--version")){puts("I/O+ 0.1.0");return 0;}struct utsname u;if(uname(&u)==0)printf("Kernel: %s %s (%s)\n",u.sysname,u.release,u.machine);list("Block devices","/sys/class/block");list("Network interfaces","/sys/class/net");list("USB devices","/sys/bus/usb/devices");list("Graphics devices","/sys/class/drm");FILE *f=fopen("/proc/meminfo","r");if(f){char b[256];if(fgets(b,sizeof b,f))printf("Memory: %s",b);fclose(f);}return 0;}
