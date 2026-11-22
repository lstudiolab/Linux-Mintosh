#include "include/xnv/arch.h"
struct xnv_idt_gate{xnv_u16 low,selector;xnv_u8 ist,attributes;xnv_u16 middle;xnv_u32 high,zero;}__attribute__((packed));
static struct xnv_idt_gate idt[256] __attribute__((aligned(16)));
static xnv_u64 gdt[3] __attribute__((aligned(16)))={0,0x00af9a000000ffffULL,0x00cf92000000ffffULL};
extern void xnv_isr_default(void);
#define DECL(n) extern void xnv_isr_##n(void)
DECL(0);DECL(1);DECL(2);DECL(3);DECL(4);DECL(5);DECL(6);DECL(7);
DECL(8);DECL(9);DECL(10);DECL(11);DECL(12);DECL(13);DECL(14);DECL(15);
DECL(16);DECL(17);DECL(18);DECL(19);DECL(20);DECL(21);DECL(22);DECL(23);
DECL(24);DECL(25);DECL(26);DECL(27);DECL(28);DECL(29);DECL(30);DECL(31);
static void(*const stubs[32])(void)={xnv_isr_0,xnv_isr_1,xnv_isr_2,xnv_isr_3,xnv_isr_4,xnv_isr_5,xnv_isr_6,xnv_isr_7,xnv_isr_8,xnv_isr_9,xnv_isr_10,xnv_isr_11,xnv_isr_12,xnv_isr_13,xnv_isr_14,xnv_isr_15,xnv_isr_16,xnv_isr_17,xnv_isr_18,xnv_isr_19,xnv_isr_20,xnv_isr_21,xnv_isr_22,xnv_isr_23,xnv_isr_24,xnv_isr_25,xnv_isr_26,xnv_isr_27,xnv_isr_28,xnv_isr_29,xnv_isr_30,xnv_isr_31};
static void set_gate(unsigned int n,void(*handler)(void))
{
    union{void(*fn)(void);xnv_u64 addr;}u;u.fn=handler;
    idt[n].low=(xnv_u16)u.addr;idt[n].selector=8;idt[n].ist=0;idt[n].attributes=0x8e;
    idt[n].middle=(xnv_u16)(u.addr>>16);idt[n].high=(xnv_u32)(u.addr>>32);idt[n].zero=0;
}
void xnv_interrupts_init(void)
{
    struct xnv_descriptor_ptr gdtr={(xnv_u16)(sizeof(gdt)-1),(xnv_u64)gdt};
    struct xnv_descriptor_ptr idtr={(xnv_u16)(sizeof(idt)-1),(xnv_u64)idt};
    for(unsigned int i=0;i<256;++i)set_gate(i,xnv_isr_default);
    for(unsigned int i=0;i<32;++i)set_gate(i,stubs[i]);
    xnv_arch_load_tables(&gdtr,&idtr);
}
void xnv_exception_dispatch(const xnv_u64 *frame)
{
    xnv_serial_write("XNV: fatal CPU exception vector ");xnv_serial_hex(frame?frame[0]:0xffff);
    xnv_serial_write("\r\n");xnv_halt();
}
