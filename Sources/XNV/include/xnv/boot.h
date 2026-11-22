#ifndef XNV_BOOT_H
#define XNV_BOOT_H
/* XNV boot handoff ABI v1. All addresses are physical; map before dereferencing. */
#define XNV_BOOT_MAGIC 0x584E56424F4F5431ULL
#define XNV_BOOT_ABI_VERSION 1U
#define XNV_BOOT_MAX_INFO_SIZE 4096U
#define XNV_BOOT_MAX_MEMORY_REGIONS 4096U

typedef unsigned short xnv_u16;
typedef unsigned int xnv_u32;
typedef unsigned long long xnv_u64;

enum xnv_boot_status {
    XNV_BOOT_OK = 0, XNV_BOOT_INVALID_ARGUMENT, XNV_BOOT_BAD_MAGIC,
    XNV_BOOT_UNSUPPORTED_VERSION, XNV_BOOT_BAD_SIZE, XNV_BOOT_BAD_MEMORY_MAP,
    XNV_BOOT_BAD_INITRD, XNV_BOOT_BAD_FRAMEBUFFER, XNV_BOOT_BAD_FIRMWARE_TABLE
};
enum xnv_memory_region_type {
    XNV_MEMORY_USABLE = 1, XNV_MEMORY_RESERVED, XNV_MEMORY_ACPI_RECLAIMABLE,
    XNV_MEMORY_ACPI_NVS, XNV_MEMORY_BAD, XNV_MEMORY_BOOTLOADER_RECLAIMABLE,
    XNV_MEMORY_KERNEL_AND_MODULES, XNV_MEMORY_FRAMEBUFFER
};
enum xnv_framebuffer_format {
    XNV_FRAMEBUFFER_NONE = 0, XNV_FRAMEBUFFER_RGBX8888, XNV_FRAMEBUFFER_BGRX8888
};
struct xnv_memory_region {
    xnv_u64 base;
    xnv_u64 length;
    xnv_u32 type;
    xnv_u32 reserved;
};
struct xnv_boot_info {
    xnv_u64 magic;
    xnv_u32 version;
    xnv_u32 size;
    xnv_u64 memory_map_address;
    xnv_u64 memory_map_entries;
    xnv_u64 initrd_address;
    xnv_u64 initrd_size;
    xnv_u64 rsdp_address;
    xnv_u64 framebuffer_address;
    xnv_u32 framebuffer_width;
    xnv_u32 framebuffer_height;
    xnv_u32 framebuffer_pitch;
    xnv_u32 framebuffer_format;
    xnv_u64 reserved[4];
};
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(struct xnv_memory_region) == 24, "XNV memory region ABI changed");
_Static_assert(sizeof(struct xnv_boot_info) == 112, "XNV boot info ABI changed");
#endif
enum xnv_boot_status xnv_boot_info_validate(const struct xnv_boot_info *info);
#endif
