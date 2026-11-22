#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "include/xnv/boot.h"
static struct xnv_boot_info valid_info(void)
{
    struct xnv_boot_info v;
    memset(&v, 0, sizeof(v));
    v.magic=XNV_BOOT_MAGIC; v.version=XNV_BOOT_ABI_VERSION; v.size=sizeof(v);
    v.memory_map_address=0x100000; v.memory_map_entries=4;
    v.initrd_address=0x400000; v.initrd_size=0x200000; v.rsdp_address=0x7df000;
    v.framebuffer_address=0xe0000000; v.framebuffer_width=1024;
    v.framebuffer_height=768; v.framebuffer_pitch=4096;
    v.framebuffer_format=XNV_FRAMEBUFFER_BGRX8888;
    return v;
}
int main(void)
{
    struct xnv_boot_info v=valid_info();
    assert(xnv_boot_info_validate(&v)==XNV_BOOT_OK);
    v.magic=0; assert(xnv_boot_info_validate(&v)==XNV_BOOT_BAD_MAGIC);
    v=valid_info(); v.version++; assert(xnv_boot_info_validate(&v)==XNV_BOOT_UNSUPPORTED_VERSION);
    v=valid_info(); v.size=1; assert(xnv_boot_info_validate(&v)==XNV_BOOT_BAD_SIZE);
    v=valid_info(); v.memory_map_address=3; assert(xnv_boot_info_validate(&v)==XNV_BOOT_BAD_MEMORY_MAP);
    v=valid_info(); v.memory_map_entries=XNV_BOOT_MAX_MEMORY_REGIONS+1ULL; assert(xnv_boot_info_validate(&v)==XNV_BOOT_BAD_MEMORY_MAP);
    v=valid_info(); v.initrd_size=0; assert(xnv_boot_info_validate(&v)==XNV_BOOT_BAD_INITRD);
    v=valid_info(); v.rsdp_address++; assert(xnv_boot_info_validate(&v)==XNV_BOOT_BAD_FIRMWARE_TABLE);
    v=valid_info(); v.framebuffer_pitch=8; assert(xnv_boot_info_validate(&v)==XNV_BOOT_BAD_FRAMEBUFFER);
    v=valid_info(); v.framebuffer_width=0xffffffffU; assert(xnv_boot_info_validate(&v)==XNV_BOOT_BAD_FRAMEBUFFER);
    v=valid_info(); v.reserved[2]=1; assert(xnv_boot_info_validate(&v)==XNV_BOOT_BAD_SIZE);
    assert(xnv_boot_info_validate(0)==XNV_BOOT_INVALID_ARGUMENT);
    puts("XNV boot ABI validation tests passed.");
    return 0;
}
