#include "include/xnv/boot.h"

#define XNV_U64_MAX (~(xnv_u64)0)

static int range_valid(xnv_u64 base, xnv_u64 length)
{
    return length != 0 && base <= XNV_U64_MAX - length;
}

enum xnv_boot_status xnv_boot_info_validate(const struct xnv_boot_info *info)
{
    xnv_u64 map_bytes;
    xnv_u64 framebuffer_bytes;

    if (info == 0)
        return XNV_BOOT_INVALID_ARGUMENT;
    if (info->magic != XNV_BOOT_MAGIC)
        return XNV_BOOT_BAD_MAGIC;
    if (info->version != XNV_BOOT_ABI_VERSION)
        return XNV_BOOT_UNSUPPORTED_VERSION;
    if (info->size < sizeof(*info) || info->size > XNV_BOOT_MAX_INFO_SIZE)
        return XNV_BOOT_BAD_SIZE;

    for (unsigned int i = 0; i < 4; ++i) {
        if (info->reserved[i] != 0)
            return XNV_BOOT_BAD_SIZE;
    }

    if (info->memory_map_address == 0 ||
        info->memory_map_entries == 0 ||
        info->memory_map_entries > XNV_BOOT_MAX_MEMORY_REGIONS ||
        info->memory_map_address % 8 != 0 ||
        info->memory_map_entries > XNV_U64_MAX / sizeof(struct xnv_memory_region))
        return XNV_BOOT_BAD_MEMORY_MAP;

    map_bytes = info->memory_map_entries * sizeof(struct xnv_memory_region);
    if (!range_valid(info->memory_map_address, map_bytes))
        return XNV_BOOT_BAD_MEMORY_MAP;

    if ((info->initrd_address == 0) != (info->initrd_size == 0))
        return XNV_BOOT_BAD_INITRD;
    if (info->initrd_size != 0 &&
        !range_valid(info->initrd_address, info->initrd_size))
        return XNV_BOOT_BAD_INITRD;

    if (info->rsdp_address != 0 && info->rsdp_address % 16 != 0)
        return XNV_BOOT_BAD_FIRMWARE_TABLE;

    if (info->framebuffer_address == 0) {
        if (info->framebuffer_width != 0 ||
            info->framebuffer_height != 0 ||
            info->framebuffer_pitch != 0 ||
            info->framebuffer_format != XNV_FRAMEBUFFER_NONE)
            return XNV_BOOT_BAD_FRAMEBUFFER;
    } else {
        if (info->framebuffer_width == 0 ||
            info->framebuffer_height == 0 ||
            info->framebuffer_pitch < info->framebuffer_width * 4U ||
            (info->framebuffer_format != XNV_FRAMEBUFFER_RGBX8888 &&
             info->framebuffer_format != XNV_FRAMEBUFFER_BGRX8888))
            return XNV_BOOT_BAD_FRAMEBUFFER;
        if (info->framebuffer_height >
            XNV_U64_MAX / info->framebuffer_pitch)
            return XNV_BOOT_BAD_FRAMEBUFFER;
        framebuffer_bytes =
            (xnv_u64)info->framebuffer_pitch * info->framebuffer_height;
        if (!range_valid(info->framebuffer_address, framebuffer_bytes))
            return XNV_BOOT_BAD_FRAMEBUFFER;
    }

    return XNV_BOOT_OK;
}
