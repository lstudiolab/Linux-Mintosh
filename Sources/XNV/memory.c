#include "include/xnv/memory.h"
#define BITMAP_BYTES (XNV_MAX_PHYSICAL_PAGES / 8ULL)
static xnv_u8 page_bitmap[BITMAP_BYTES];
static xnv_u64 free_pages;
static int bit_is_set(xnv_u64 p) { return (page_bitmap[p>>3] & (xnv_u8)(1U<<(p&7))) != 0; }
static void bit_set(xnv_u64 p) { page_bitmap[p>>3] |= (xnv_u8)(1U<<(p&7)); }
static void bit_clear(xnv_u64 p) { page_bitmap[p>>3] &= (xnv_u8)~(1U<<(p&7)); }
enum xnv_memory_status xnv_phys_init(const struct xnv_memory_region *r,xnv_u64 n)
{
    if(!r||!n||n>XNV_BOOT_MAX_MEMORY_REGIONS) return XNV_MEMORY_INVALID_MAP;
    for(xnv_u64 i=0;i<BITMAP_BYTES;++i) page_bitmap[i]=0xff;
    free_pages=0;
    for(xnv_u64 i=0;i<n;++i) {
        xnv_u64 end,first,last;
        if(!r[i].length||r[i].base>(~(xnv_u64)0)-r[i].length) return XNV_MEMORY_INVALID_MAP;
        if(r[i].type!=XNV_MEMORY_USABLE) continue;
        end=r[i].base+r[i].length;
        first=(r[i].base+XNV_PAGE_SIZE-1)/XNV_PAGE_SIZE;
        last=end/XNV_PAGE_SIZE;
        if(first<256) first=256;
        if(last>XNV_MAX_PHYSICAL_PAGES) last=XNV_MAX_PHYSICAL_PAGES;
        for(xnv_u64 p=first;p<last;++p) if(bit_is_set(p)){bit_clear(p);++free_pages;}
    }
    return XNV_MEMORY_OK;
}
enum xnv_memory_status xnv_phys_alloc(xnv_u64 *address)
{
    if(!address) return XNV_MEMORY_INVALID_MAP;
    for(xnv_u64 p=256;p<XNV_MAX_PHYSICAL_PAGES;++p) if(!bit_is_set(p)){
        bit_set(p);--free_pages;*address=p*XNV_PAGE_SIZE;return XNV_MEMORY_OK;
    }
    return XNV_MEMORY_EXHAUSTED;
}
enum xnv_memory_status xnv_phys_free(xnv_u64 address)
{
    xnv_u64 p;
    if(address%XNV_PAGE_SIZE) return XNV_MEMORY_INVALID_FREE;
    p=address/XNV_PAGE_SIZE;
    if(p<256||p>=XNV_MAX_PHYSICAL_PAGES||!bit_is_set(p)) return XNV_MEMORY_INVALID_FREE;
    bit_clear(p);++free_pages;return XNV_MEMORY_OK;
}
xnv_u64 xnv_phys_free_pages(void){return free_pages;}
