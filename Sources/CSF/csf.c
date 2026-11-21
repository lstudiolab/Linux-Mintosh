#include "csf.h"
#include "libsys.h"
int csf_request(const char *q,char *r,size_t n){return ms_send_command(MINTOSH_MOSF_SOCKET,q,r,n);}
