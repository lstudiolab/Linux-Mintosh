#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "include/xnv/boot.h"

static struct xnv_boot_info valid_info(void)
{
    struct xnv_boot_info info;
    memset(&info, 0, sizeof(info));
    info.magic = XNV_BOOT_MAGIC;
    info.version = XNV_BOOT_ABI_VERSION;
    info.size = sizeof(info);
    info.memory_map_address = 0x100000;
    info.memory_map_entries = 4;
    info.initrd_address = 0x400000;
    info.initrd_size = 0x200000;
    info.rsdp_address = 0x7df000;
    info.framebuffer_address = 0xe0000000;
    info.framebuffer_width = 1024;
    info.framebuffer_height = 768;
    info.framebuffer_pitch = 4096;
    info.framebuffer_format = XNV_FRAMEBUFFER_BGRX8888;
    return info;
}

int main(void)
{
    struct xnv_boot_info info = valid_info();
    assert(xnv_boot_info_validate(&info) == XNV_BOOT_OK);

    info.magic = 0;
    assert(xnv_boot_info_validate(&info) == XNV_BOOT_BAD_MAGIC);
    info = valid_info();
    info.version++;
    assert(xnv_boot_info_validate(&info) == XNV_BOOT_UNSUPPORTED_VERSION);
    info = valid_info();
    info.size = 1;
    assert(xnv_boot_info_validate(&info) == XNV_BOOT_BAD_SIZE);
    info = valid_info();
    info.memory_map_address = 3;
    assert(xnv_boot_info_validate(&info) == XNV_BOOT_BAD_MEMORY_MAP);
    info = valid_info();
    info.memory_map_entries = XNV_BOOT_MAX_MEMORY_REGIONS + 1ULL;
    assert(xnv_boot_info_validate(&info) == XNV_BOOT_BAD_MEMORY_MAP);
    info = valid_info();
    info.initrd_size = 0;
    assert(xnv_boot_info_validate(&info) == XNV_BOOT_BAD_INITRD);
    info = valid_info();
    info.rsdp_address++;
    assert(xnv_boot_info_validate(&info) == XNV_BOOT_BAD_FIRMWARE_TABLE);
    info = valid_info();
    info.framebuffer_pitch = 8;
    assert(xnv_boot_info_validate(&info) == XNV_BOOT_BAD_FRAMEBUFFER);
    info = valid_info();
    info.reserved[2] = 1;
    assert(xnv_boot_info_validate(&info) == XNV_BOOT_BAD_SIZE);
    assert(xnv_boot_info_validate(0) == XNV_BOOT_INVALID_ARGUMENT);

    puts("XNV boot ABI validation tests passed.");
    return 0;
}
