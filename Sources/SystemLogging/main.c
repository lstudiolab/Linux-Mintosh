#include "libsys.h"
#include <stdio.h>
#include <string.h>
int main(int argc,char **argv){if(argc==2&&!strcmp(argv[1],"--version")){puts("Mintosh System Logging 0.1.0");return 0;}if(argc<3){fprintf(stderr,"usage: mosflog info|warning|error MESSAGE\n");return 64;}const char *l=argv[1];if(strcmp(l,"info")&&strcmp(l,"warning")&&strcmp(l,"error")&&strcmp(l,"debug")){fputs("mosflog: invalid level\n",stderr);return 64;}ms_log("mintosh",l,argv[2]);return 0;}
