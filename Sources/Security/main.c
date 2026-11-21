#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
static int check(const char *p){struct stat s;if(stat(p,&s)<0){perror(p);return 1;}if(s.st_mode&0022){fprintf(stderr,"WARNING: %s writable by group/other (%04o)\n",p,s.st_mode&07777);return 1;}printf("OK: %s (%04o)\n",p,s.st_mode&07777);return 0;}
int main(int argc,char **argv){if(argc>1&&!strcmp(argv[1],"--version")){puts("Mintosh Security Audit 0.1.0");return 0;}if(geteuid()!=0){fputs("security-audit: run as root\n",stderr);return 2;}int e=check("/etc/shadow")+check("/etc/passwd")+check("/etc/sudoers");puts(e?"Audit completed with warnings.":"Basic permission audit passed.");return e?1:0;}
