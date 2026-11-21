#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
static int run(char *const a[]){execvp(a[0],a);fprintf(stderr,"kextctl: %s: %s\n",a[0],strerror(errno));return errno==ENOENT?127:1;}
int main(int argc,char **argv){if(argc==2&&!strcmp(argv[1],"--version")){puts("Kexts 0.1.0");return 0;}if(argc==2&&!strcmp(argv[1],"list")){char *a[]={"lsmod",NULL};return run(a);}if(argc==3&&!strcmp(argv[1],"load")){char *a[]={"modprobe",argv[2],NULL};return run(a);}if(argc==3&&!strcmp(argv[1],"unload")){char *a[]={"modprobe","-r",argv[2],NULL};return run(a);}fprintf(stderr,"usage: kextctl list|load MODULE|unload MODULE\n");return 64;}
