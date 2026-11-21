#include "Foundation.h"
#include <stdlib.h>
#include <string.h>
MFString mf_string_copy(const char *s){MFString v={0};if(!s)return v;v.length=strlen(s);v.data=malloc(v.length+1);if(v.data)memcpy(v.data,s,v.length+1);else v.length=0;return v;}
void mf_string_destroy(MFString *v){if(v){free(v->data);v->data=NULL;v->length=0;}}
int mf_string_equal(const MFString *a,const MFString *b){return a&&b&&a->data&&b->data&&a->length==b->length&&!memcmp(a->data,b->data,a->length);}
char *mf_path_join(const char *a,const char *b){if(!a||!b)return NULL;size_t x=strlen(a),y=strlen(b);int slash=x&&a[x-1]!='/';char *p=malloc(x+y+(size_t)slash+1);if(!p)return NULL;memcpy(p,a,x);if(slash)p[x++]='/';memcpy(p+x,b,y+1);return p;}
