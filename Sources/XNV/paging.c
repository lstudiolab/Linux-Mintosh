#include "include/xnv/arch.h"
#include "include/xnv/memory.h"
#define P 0x001ULL
#define W 0x002ULL
#define PS 0x080ULL
#define LARGE_PAGE_SIZE 0x200000ULL
static xnv_u64 pml4[512] __attribute__((aligned(4096)));
static xnv_u64 pdpt[512] __attribute__((aligned(4096)));
static xnv_u64 pds[32][512] __attribute__((aligned(4096)));
static void zero_table(xnv_u64 *p){for(unsigned int i=0;i<512;++i)p[i]=0;}
static int ram_type(xnv_u32 t)
{
    return t==XNV_MEMORY_USABLE||t==XNV_MEMORY_ACPI_RECLAIMABLE||
           t==XNV_MEMORY_ACPI_NVS||t==XNV_MEMORY_BOOTLOADER_RECLAIMABLE||
           t==XNV_MEMORY_KERNEL_AND_MODULES;
}
static int range_is_ram(xnv_u64 start,xnv_u64 end,
                        const struct xnv_memory_region *r,xnv_u64 count)
{
    xnv_u64 low=0,high=count,cursor=start;
    while(low<high){
        xnv_u64 mid=low+(high-low)/2;
        if(r[mid].base+r[mid].length<=start)low=mid+1;else high=mid;
    }
    for(xnv_u64 i=low;i<count&&cursor<end;++i){
        xnv_u64 rend=r[i].base+r[i].length;
        if(r[i].base>cursor||!ram_type(r[i].type))return 0;
        if(rend>cursor)cursor=rend;
    }
    return cursor>=end;
}
int xnv_paging_init(const struct xnv_memory_region *regions,xnv_u64 count)
{
    xnv_u64 cr4;
    if(!regions||!count)return 0;
    __asm__ volatile("mov %%cr4,%0":"=r"(cr4));
    if(cr4&(1ULL<<12))return 0;
    zero_table(pml4);zero_table(pdpt);pml4[0]=(xnv_u64)pdpt|P|W;
    for(unsigned int g=0;g<32;++g){
        zero_table(pds[g]);pdpt[g]=(xnv_u64)pds[g]|P|W;
        for(unsigned int m=0;m<512;++m){
            xnv_u64 base=((xnv_u64)g*512+m)*LARGE_PAGE_SIZE;
            if(range_is_ram(base,base+LARGE_PAGE_SIZE,regions,count)){
                pds[g][m]=base|P|W|PS;
            }else{
                xnv_u64 table_phys;
                if(xnv_phys_alloc(&table_phys)!=XNV_MEMORY_OK)return 0;
                xnv_u64 *pt=(xnv_u64*)(UINTN)table_phys;
                int populated=0;
                for(unsigned int p=0;p<512;++p){
                    xnv_u64 page=base+(xnv_u64)p*XNV_PAGE_SIZE;
                    if(range_is_ram(page,page+XNV_PAGE_SIZE,regions,count)){
                        pt[p]=page|P|W;populated=1;
                    }else pt[p]=0;
                }
                if(populated)pds[g][m]=table_phys|P|W;
                else xnv_phys_free(table_phys);
            }
        }
    }
    xnv_arch_enable_paging((xnv_u64)pml4);
    return 1;
}
