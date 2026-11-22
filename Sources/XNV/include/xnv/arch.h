#ifndef XNV_ARCH_H
#define XNV_ARCH_H
#include "boot.h"
struct xnv_descriptor_ptr { xnv_u16 limit; xnv_u64 base; } __attribute__((packed));
void xnv_arch_enter(struct xnv_boot_info *info,xnv_u64 stack_top) __attribute__((noreturn));
void xnv_arch_load_tables(const struct xnv_descriptor_ptr *gdtr,const struct xnv_descriptor_ptr *idtr);
void xnv_arch_enable_paging(xnv_u64 pml4_physical);
void xnv_serial_init(void);
void xnv_serial_write(const char *text);
void xnv_serial_hex(xnv_u64 value);
void xnv_halt(void) __attribute__((noreturn));
void xnv_interrupts_init(void);
int xnv_paging_init(void);
#endif
