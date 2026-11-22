#include "include/xnv/arch.h"
#define P 0x001ULL
#define W 0x002ULL
#define PS 0x080ULL
static xnv_u64 pml4[512] __attribute__((aligned(4096)));
static xnv_u64 pdpt[512] __attribute__((aligned(4096)));
static xnv_u64 pds[32][512] __attribute__((aligned(4096)));
static void zero_table(xnv_u64 *p){for(unsigned int i=0;i<512;++i)p[i]=0;}
int xnv_paging_init(void)
{
    xnv_u64 cr4;
    __asm__ volatile("mov %%cr4,%0":"=r"(cr4));
    if(cr4&(1ULL<<12)) return 0;
    zero_table(pml4);zero_table(pdpt);pml4[0]=(xnv_u64)pdpt|P|W;
    for(unsigned int g=0;g<32;++g){zero_table(pds[g]);pdpt[g]=(xnv_u64)pds[g]|P|W;
        for(unsigned int m=0;m<512;++m){xnv_u64 a=((xnv_u64)g*512+m)*0x200000ULL;pds[g][m]=a|P|W|PS;}}
    xnv_arch_enable_paging((xnv_u64)pml4);
    return 1;
}
