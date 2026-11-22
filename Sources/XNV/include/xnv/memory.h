#ifndef XNV_MEMORY_H
#define XNV_MEMORY_H
#include "boot.h"
#define XNV_PAGE_SIZE 4096ULL
#define XNV_MAX_PHYSICAL_PAGES 8388608ULL
enum xnv_memory_status { XNV_MEMORY_OK=0, XNV_MEMORY_INVALID_MAP, XNV_MEMORY_EXHAUSTED, XNV_MEMORY_INVALID_FREE };
enum xnv_memory_status xnv_phys_init(const struct xnv_memory_region *regions, xnv_u64 count);
enum xnv_memory_status xnv_phys_alloc(xnv_u64 *physical_address);
enum xnv_memory_status xnv_phys_free(xnv_u64 physical_address);
xnv_u64 xnv_phys_free_pages(void);
#endif
