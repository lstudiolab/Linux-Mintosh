#include "include/xnv/arch.h"
static void outb(xnv_u16 p,xnv_u8 v){__asm__ volatile("outb %0,%1"::"a"(v),"Nd"(p));}
static xnv_u8 inb(xnv_u16 p){xnv_u8 v;__asm__ volatile("inb %1,%0":"=a"(v):"Nd"(p));return v;}
void xnv_serial_init(void){outb(0x3f9,0);outb(0x3fb,0x80);outb(0x3f8,1);outb(0x3f9,0);outb(0x3fb,3);outb(0x3fa,0xc7);outb(0x3fc,0x0b);}
void xnv_serial_write(const char *s){if(!s)return;while(*s){while((inb(0x3fd)&0x20)==0){}outb(0x3f8,(xnv_u8)*s++);}}
void xnv_serial_hex(xnv_u64 v){static const char d[]="0123456789ABCDEF";char b[19]="0x0000000000000000";for(unsigned int i=0;i<16;++i)b[2+i]=d[(v>>((15-i)*4))&15];xnv_serial_write(b);}
void xnv_halt(void){__asm__ volatile("cli");for(;;)__asm__ volatile("hlt");}
