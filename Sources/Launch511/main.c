#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
int main(int argc,char **argv){int i=1;if(i<argc&&!strcmp(argv[i],"--"))i++;if(i>=argc){fprintf(stderr,"usage: launch511 command [arguments...]\n");return 64;}execvp(argv[i],&argv[i]);fprintf(stderr,"launch511: %s: %s\n",argv[i],strerror(errno));return errno==ENOENT?127:126;}
