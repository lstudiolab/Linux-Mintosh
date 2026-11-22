#include <efi.h>
#include <efilib.h>
#include "include/xnv/boot.h"
#include "include/xnv/arch.h"
#include "include/xnv/memory.h"
#define XNV_STACK_SIZE (64U*1024U)
#define XNV_DIRECT_LIMIT (32ULL*1024ULL*1024ULL*1024ULL)
static xnv_u8 kernel_stack[XNV_STACK_SIZE] __attribute__((aligned(16)));
static xnv_u32 region_type(UINT32 type)
{
    switch(type){
    case EfiConventionalMemory:return XNV_MEMORY_USABLE;
    case EfiACPIReclaimMemory:return XNV_MEMORY_ACPI_RECLAIMABLE;
    case EfiACPIMemoryNVS:return XNV_MEMORY_ACPI_NVS;
    case EfiUnusableMemory:return XNV_MEMORY_BAD;
    case EfiLoaderCode:case EfiLoaderData:case EfiBootServicesCode:case EfiBootServicesData:
        return XNV_MEMORY_BOOTLOADER_RECLAIMABLE;
    default:return XNV_MEMORY_RESERVED;
    }
}
static EFI_STATUS alloc_pool(UINTN size,VOID **out)
{return uefi_call_wrapper(BS->AllocatePool,3,EfiLoaderData,size,out);}
static int below_direct_map(UINTN p,UINTN size)
{return (xnv_u64)p<XNV_DIRECT_LIMIT&&(xnv_u64)size<XNV_DIRECT_LIMIT-(xnv_u64)p;}
static EFI_STATUS prepare_handoff(EFI_HANDLE image,struct xnv_boot_info **out)
{
    EFI_STATUS status;
    UINTN needed=0,map_size,desc_size=0,key=0;
    UINT32 version=0;
    VOID *map=0;
    struct xnv_boot_info *info=0;
    struct xnv_memory_region *regions=0;
    status=uefi_call_wrapper(BS->GetMemoryMap,5,&needed,0,&key,&desc_size,&version);
    if(status!=EFI_BUFFER_TOO_SMALL||!desc_size)return EFI_LOAD_ERROR;
    map_size=needed+desc_size*32;
    status=alloc_pool(map_size,&map);if(EFI_ERROR(status))return status;
    status=alloc_pool(sizeof(*info),(VOID**)&info);if(EFI_ERROR(status))return status;
    status=alloc_pool(sizeof(*regions)*XNV_BOOT_MAX_MEMORY_REGIONS,(VOID**)&regions);
    if(EFI_ERROR(status))return status;
    if(!below_direct_map((UINTN)map,map_size)||
       !below_direct_map((UINTN)info,sizeof(*info))||
       !below_direct_map((UINTN)regions,sizeof(*regions)*XNV_BOOT_MAX_MEMORY_REGIONS)||
       !below_direct_map((UINTN)kernel_stack,sizeof(kernel_stack)))return EFI_UNSUPPORTED;
    for(unsigned int attempt=0;attempt<4;++attempt){
        UINTN actual=map_size,count=0;
        status=uefi_call_wrapper(BS->GetMemoryMap,5,&actual,map,&key,&desc_size,&version);
        if(status==EFI_BUFFER_TOO_SMALL)return status;
        if(EFI_ERROR(status))return status;
        for(UINTN off=0;off<actual;off+=desc_size){
            EFI_MEMORY_DESCRIPTOR *d=(EFI_MEMORY_DESCRIPTOR*)((UINT8*)map+off);
            xnv_u64 length;
            if(!d->NumberOfPages)continue;
            if(d->NumberOfPages>(~(xnv_u64)0)/EFI_PAGE_SIZE)return EFI_LOAD_ERROR;
            length=(xnv_u64)d->NumberOfPages*EFI_PAGE_SIZE;
            if(d->PhysicalStart>(~(xnv_u64)0)-length)return EFI_LOAD_ERROR;
            if(count>=XNV_BOOT_MAX_MEMORY_REGIONS)return EFI_OUT_OF_RESOURCES;
            regions[count].base=d->PhysicalStart;
            regions[count].length=length;
            regions[count].type=region_type(d->Type);
            regions[count].reserved=0;
            ++count;
        }
        for(UINTN i=0;i<sizeof(*info);++i)((UINT8*)info)[i]=0;
        info->magic=XNV_BOOT_MAGIC;info->version=XNV_BOOT_ABI_VERSION;info->size=sizeof(*info);
        info->memory_map_address=(xnv_u64)(UINTN)regions;info->memory_map_entries=count;
        if(xnv_boot_info_validate(info)!=XNV_BOOT_OK)return EFI_LOAD_ERROR;
        status=uefi_call_wrapper(BS->ExitBootServices,2,image,key);
        if(!EFI_ERROR(status)){*out=info;return EFI_SUCCESS;}
    }
    return status;
}
void xnv_kernel_main(struct xnv_boot_info *info)
{
    xnv_serial_init();
    xnv_serial_write("\r\nMintosh XNV kernel starting\r\n");
    if(xnv_boot_info_validate(info)!=XNV_BOOT_OK){xnv_serial_write("XNV: invalid boot handoff\r\n");xnv_halt();}
    xnv_serial_write("XNV: UEFI boot ABI v1 accepted\r\n");
    xnv_interrupts_init();
    xnv_serial_write("XNV: GDT and IDT installed; exceptions fail-stop\r\n");
    if(xnv_phys_init((const struct xnv_memory_region*)(UINTN)info->memory_map_address,
                     info->memory_map_entries)!=XNV_MEMORY_OK){
        xnv_serial_write("XNV: physical memory map rejected\r\n");xnv_halt();
    }
    xnv_serial_write("XNV: physical page allocator ready; free pages=");
    xnv_serial_hex(xnv_phys_free_pages());xnv_serial_write("\r\n");
    xnv_serial_write("XNV: enabling initial 32 GiB identity map\r\n");
    if(!xnv_paging_init()){xnv_serial_write("XNV: unsupported five-level paging mode\r\n");xnv_halt();}
    xnv_serial_write("XNV: paging enabled; boot services exited\r\n");
    xnv_serial_write("XNV: bootstrap reached idle state\r\n");
    xnv_halt();
}
EFI_STATUS EFIAPI efi_main(EFI_HANDLE image_handle,EFI_SYSTEM_TABLE *system_table)
{
    EFI_STATUS status;
    struct xnv_boot_info *info=0;
    EFI_LOADED_IMAGE *loaded=0;
    InitializeLib(image_handle,system_table);
    status=uefi_call_wrapper(BS->HandleProtocol,3,image_handle,&LoadedImageProtocol,(VOID**)&loaded);
    if(EFI_ERROR(status)||!loaded||!below_direct_map((UINTN)loaded->ImageBase,(UINTN)loaded->ImageSize)){
        Print(L"XNV: kernel image is outside the initial direct-map range.\r\n");
        return EFI_UNSUPPORTED;
    }
    status=prepare_handoff(image_handle,&info);
    if(EFI_ERROR(status)){
        Print(L"XNV: failed to prepare boot handoff or exit boot services: %r\r\n",status);
        return status;
    }
    xnv_arch_enter(info,(xnv_u64)(UINTN)(kernel_stack+sizeof(kernel_stack)));
}
