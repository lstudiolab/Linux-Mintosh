#define _GNU_SOURCE
#include "libsys.h"
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/stat.h>
int ms_mkdir_p(const char *path, mode_t mode) { char b[4096]; size_t n=strlen(path); if(!n||n>=sizeof b){errno=ENAMETOOLONG;return -1;} memcpy(b,path,n+1); for(char *p=b+1;*p;p++) if(*p=='/'){*p=0;if(mkdir(b,mode)&&errno!=EEXIST)return -1;*p='/';} return mkdir(b,mode)&&errno!=EEXIST?-1:0; }
int ms_write_all(int fd,const void *data,size_t len){const char *p=data;while(len){ssize_t n=write(fd,p,len);if(n<0){if(errno==EINTR)continue;return -1;}p+=n;len-=(size_t)n;}return 0;}
int ms_send_command(const char *path,const char *cmd,char *reply,size_t cap){int fd=socket(AF_UNIX,SOCK_STREAM|SOCK_CLOEXEC,0);if(fd<0)return -1;struct sockaddr_un a={.sun_family=AF_UNIX};if(strlen(path)>=sizeof a.sun_path){close(fd);errno=ENAMETOOLONG;return -1;}strcpy(a.sun_path,path);if(connect(fd,(struct sockaddr*)&a,sizeof a)<0){int e=errno;close(fd);errno=e;return -1;}if(ms_write_all(fd,cmd,strlen(cmd))<0||ms_write_all(fd,"\n",1)<0){close(fd);return -1;}ssize_t n=read(fd,reply,cap-1);int e=errno;close(fd);errno=e;if(n<0)return -1;reply[n]=0;return (int)n;}
void ms_log(const char *component,const char *level,const char *message){int p=LOG_INFO;if(!strcmp(level,"error"))p=LOG_ERR;else if(!strcmp(level,"warning"))p=LOG_WARNING;else if(!strcmp(level,"debug"))p=LOG_DEBUG;openlog(component,LOG_PID|LOG_NDELAY,LOG_DAEMON);syslog(p,"%s",message);closelog();}
