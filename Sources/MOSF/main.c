#define _GNU_SOURCE
#include "libsys.h"
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>
static volatile sig_atomic_t stopping;
static void stop_signal(int sig){(void)sig;stopping=1;}
int main(int argc,char **argv){
 if(argc>1&&!strcmp(argv[1],"--version")){puts("MOSF 0.1.0");return 0;}
 if(getpid()==1){fputs("MOSF is a userspace foundation daemon, not PID 1.\n",stderr);return 2;}
 if(ms_mkdir_p(MINTOSH_RUNTIME_DIR,0755)<0){perror("mosf: runtime directory");return 1;}
 unlink(MINTOSH_MOSF_SOCKET);
 int server=socket(AF_UNIX,SOCK_STREAM|SOCK_CLOEXEC,0);if(server<0){perror("mosf: socket");return 1;}
 struct sockaddr_un a={.sun_family=AF_UNIX};strcpy(a.sun_path,MINTOSH_MOSF_SOCKET);
 if(bind(server,(struct sockaddr*)&a,sizeof a)<0||chmod(MINTOSH_MOSF_SOCKET,0666)<0||listen(server,16)<0){perror("mosf: listen");close(server);return 1;}
 signal(SIGTERM,stop_signal);signal(SIGINT,stop_signal);ms_log("mosf","info","MOSF started");
 while(!stopping){int c=accept4(server,NULL,NULL,SOCK_CLOEXEC);if(c<0){if(errno==EINTR)continue;if(stopping)break;continue;}char q[128]={0},r[256];ssize_t n=read(c,q,sizeof q-1);if(n<0)n=0;q[strcspn(q,"\r\n")]=0;
  if(!strcmp(q,"PING"))snprintf(r,sizeof r,"OK MOSF 0.1.0\n");
  else if(!strcmp(q,"VERSION"))snprintf(r,sizeof r,"Mintosh 0.1.0; MOSF 0.1.0\n");
  else if(!strcmp(q,"STATUS"))snprintf(r,sizeof r,"MOSF=running\n");
  else snprintf(r,sizeof r,"ERROR unsupported command\n");
  ms_write_all(c,r,strlen(r));close(c);
 }
 close(server);unlink(MINTOSH_MOSF_SOCKET);ms_log("mosf","info","MOSF stopped");return 0;
}
