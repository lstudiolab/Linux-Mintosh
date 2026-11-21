#define _GNU_SOURCE
#include "libsys.h"
#include <dirent.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>
#define LIMIT 64
typedef struct{char name[64],exec[512];int enabled,restart;pid_t pid;} Service;
static Service services[LIMIT];static size_t count;static volatile sig_atomic_t stopping;
static void stop_signal(int s){(void)s;stopping=1;}
static Service *find(const char *n){for(size_t i=0;i<count;i++)if(!strcmp(n,services[i].name))return &services[i];return NULL;}
static void start(Service *s){if(!s||s->pid>0||!s->exec[0])return;pid_t p=fork();if(p<0){ms_log("launch7762","error","fork failed");return;}if(!p){setsid();execl("/usr/bin/launch511","launch511","/bin/sh","-c",s->exec,(char*)NULL);_exit(127);}s->pid=p;char b[128];snprintf(b,sizeof b,"started %s pid=%ld",s->name,(long)p);ms_log("launch7762","info",b);}
static void stop(Service *s){if(s){s->enabled=0;if(s->pid>0){kill(-s->pid,SIGTERM);kill(s->pid,SIGTERM);}}}
static void load(void){DIR *d=opendir("/etc/mintosh/services");if(!d)return;struct dirent *e;while((e=readdir(d))&&count<LIMIT){size_t n=strlen(e->d_name);if(n<=8||strcmp(e->d_name+n-8,".service"))continue;char path[256];snprintf(path,sizeof path,"/etc/mintosh/services/%s",e->d_name);FILE *f=fopen(path,"r");if(!f)continue;Service s={0};snprintf(s.name,sizeof s.name,"%.*s",(int)(n-8),e->d_name);char line[600];while(fgets(line,sizeof line,f)){char *v=strchr(line,'=');if(!v)continue;*v++=0;line[strcspn(line,"\r\n")]=0;v[strcspn(v,"\r\n")]=0;if(!strcmp(line,"ExecStart"))snprintf(s.exec,sizeof s.exec,"%s",v);else if(!strcmp(line,"Enabled"))s.enabled=!strcmp(v,"yes");else if(!strcmp(line,"Restart"))s.restart=!strcmp(v,"always")||!strcmp(v,"on-failure");}fclose(f);if(s.exec[0])services[count++]=s;}closedir(d);}
static void reap(void){int st;pid_t p;while((p=waitpid(-1,&st,WNOHANG))>0)for(size_t i=0;i<count;i++)if(services[i].pid==p){services[i].pid=0;char b[128];snprintf(b,sizeof b,"%s exited status=%d",services[i].name,st);ms_log("launch7762","warning",b);if(services[i].restart&&services[i].enabled&&!stopping)start(&services[i]);}}
static void send_reply(int fd,const char *s){ms_write_all(fd,s,strlen(s));}
static void handle(int fd){char q[256]={0};ssize_t n=read(fd,q,sizeof q-1);if(n<=0)return;q[strcspn(q,"\r\n")]=0;if(!strcmp(q,"LIST")){char b[160];for(size_t i=0;i<count;i++){snprintf(b,sizeof b,"%s %s pid=%ld\n",services[i].name,services[i].pid?"running":"stopped",(long)services[i].pid);send_reply(fd,b);}send_reply(fd,"END\n");return;}char action[24]={0},name[64]={0};if(sscanf(q,"%23s %63s",action,name)!=2){send_reply(fd,"ERROR invalid command\n");return;}Service *s=find(name);if(!s){send_reply(fd,"ERROR unknown service\n");return;}if(!strcmp(action,"START")){start(s);send_reply(fd,"OK\n");}else if(!strcmp(action,"STOP")){stop(s);send_reply(fd,"OK\n");}else if(!strcmp(action,"RESTART")){stop(s);send_reply(fd,"OK restart requested\n");}else send_reply(fd,"ERROR unsupported command\n");}
int main(int argc,char **argv){if(argc>1&&!strcmp(argv[1],"--version")){puts("launch7762 0.1.0");return 0;}if(ms_mkdir_p(MINTOSH_RUNTIME_DIR,0755)<0){perror("launch7762");return 1;}load();unlink(MINTOSH_LAUNCH_SOCKET);int server=socket(AF_UNIX,SOCK_STREAM|SOCK_CLOEXEC,0);if(server<0){perror("launch7762 socket");return 1;}struct sockaddr_un a={.sun_family=AF_UNIX};strcpy(a.sun_path,MINTOSH_LAUNCH_SOCKET);if(bind(server,(struct sockaddr*)&a,sizeof a)<0||chmod(MINTOSH_LAUNCH_SOCKET,0660)<0||listen(server,16)<0){perror("launch7762 listen");return 1;}signal(SIGTERM,stop_signal);signal(SIGINT,stop_signal);ms_log("launch7762","info","Mintosh service manager started");for(size_t i=0;i<count;i++)if(services[i].enabled)start(&services[i]);while(!stopping){fd_set set;FD_ZERO(&set);FD_SET(server,&set);struct timeval tv={.tv_sec=1};int r=select(server+1,&set,NULL,NULL,&tv);if(r>0&&FD_ISSET(server,&set)){int c=accept4(server,NULL,NULL,SOCK_CLOEXEC);if(c>=0){handle(c);close(c);}}reap();}for(size_t i=0;i<count;i++)stop(&services[i]);for(int i=0;i<10;i++){reap();usleep(100000);}close(server);unlink(MINTOSH_LAUNCH_SOCKET);return 0;}
