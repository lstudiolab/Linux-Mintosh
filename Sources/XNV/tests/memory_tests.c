#include <assert.h>
#include <stdio.h>
#include "include/xnv/memory.h"
int main(void)
{
    struct xnv_memory_region map[]={{0,0x9f000,XNV_MEMORY_USABLE,0},
      {0x100000,0x3f00000,XNV_MEMORY_USABLE,0},{0x4000000,0x100000,XNV_MEMORY_RESERVED,0}};
    xnv_u64 a,b;
    assert(xnv_phys_init(map,3)==XNV_MEMORY_OK);
    assert(xnv_phys_free_pages()==16128);
    assert(xnv_phys_alloc(&a)==XNV_MEMORY_OK&&a==0x100000);
    assert(xnv_phys_alloc(&b)==XNV_MEMORY_OK&&b==0x101000);
    assert(xnv_phys_free(a)==XNV_MEMORY_OK);
    assert(xnv_phys_free(a)==XNV_MEMORY_INVALID_FREE);
    assert(xnv_phys_free(b+1)==XNV_MEMORY_INVALID_FREE);
    assert(xnv_phys_free(b)==XNV_MEMORY_OK);
    assert(xnv_phys_init(0,0)==XNV_MEMORY_INVALID_MAP);
    puts("XNV physical allocator tests passed.");return 0;
}
