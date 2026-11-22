#include <efi.h>
#include <efilib.h>

#define MBOOT_KERNEL_PATH L"\\boot\\vmlinuz"
static CHAR16 mboot_kernel_options[] =
    L"console=tty0 console=ttyS0,115200n8 loglevel=4 rdinit=/init "
    L"initrd=\\boot\\initramfs.cpio.gz";

static EFI_STATUS read_file(EFI_FILE_HANDLE root, CHAR16 *path,
                            VOID **buffer, UINTN *size) {
    EFI_STATUS status;
    EFI_FILE_HANDLE file = NULL;
    EFI_FILE_INFO *info = NULL;
    UINTN info_size = 0;
    UINTN read_size;

    status = uefi_call_wrapper(root->Open, 5, root, &file, path,
                               EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(status)) return status;

    status = uefi_call_wrapper(file->GetInfo, 4, file, &gEfiFileInfoGuid,
                               &info_size, NULL);
    if (status == EFI_BUFFER_TOO_SMALL) {
        info = AllocatePool(info_size);
        if (info == NULL) {
            uefi_call_wrapper(file->Close, 1, file);
            return EFI_OUT_OF_RESOURCES;
        }
        status = uefi_call_wrapper(file->GetInfo, 4, file, &gEfiFileInfoGuid,
                                   &info_size, info);
    }
    if (EFI_ERROR(status) || info == NULL || info->FileSize == 0 ||
        info->FileSize > 1024ULL * 1024ULL * 1024ULL) {
        if (info != NULL) FreePool(info);
        uefi_call_wrapper(file->Close, 1, file);
        return EFI_LOAD_ERROR;
    }

    *size = (UINTN)info->FileSize;
    FreePool(info);
    *buffer = AllocatePool(*size);
    if (*buffer == NULL) {
        uefi_call_wrapper(file->Close, 1, file);
        return EFI_OUT_OF_RESOURCES;
    }

    read_size = *size;
    status = uefi_call_wrapper(file->Read, 3, file, &read_size, *buffer);
    uefi_call_wrapper(file->Close, 1, file);
    if (EFI_ERROR(status) || read_size != *size) {
        FreePool(*buffer);
        *buffer = NULL;
        return EFI_LOAD_ERROR;
    }
    return EFI_SUCCESS;
}

static EFI_STATUS validate_gzip_file(EFI_FILE_HANDLE root,
                                       CHAR16 *path) {
    EFI_STATUS status;
    EFI_FILE_HANDLE file = NULL;
    EFI_FILE_INFO *info = NULL;
    UINTN info_size = 0;
    UINTN header_size = 10;
    UINT8 header[10];

    status = uefi_call_wrapper(root->Open, 5, root, &file, path,
                               EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(status)) return status;

    status = uefi_call_wrapper(file->GetInfo, 4, file, &gEfiFileInfoGuid,
                               &info_size, NULL);
    if (status == EFI_BUFFER_TOO_SMALL) {
        info = AllocatePool(info_size);
        if (info == NULL) {
            uefi_call_wrapper(file->Close, 1, file);
            return EFI_OUT_OF_RESOURCES;
        }
        status = uefi_call_wrapper(file->GetInfo, 4, file, &gEfiFileInfoGuid,
                                   &info_size, info);
    }

    if (EFI_ERROR(status) || info == NULL || info->FileSize < 18 ||
        info->FileSize > 1024ULL * 1024ULL * 1024ULL) {
        if (info != NULL) FreePool(info);
        uefi_call_wrapper(file->Close, 1, file);
        return EFI_LOAD_ERROR;
    }
    FreePool(info);

    status = uefi_call_wrapper(file->Read, 3, file, &header_size, header);
    uefi_call_wrapper(file->Close, 1, file);
    if (EFI_ERROR(status) || header_size != sizeof(header))
        return EFI_LOAD_ERROR;

    /* RFC 1952: gzip magic, DEFLATE method, and no reserved flag bits. */
    if (header[0] != 0x1f || header[1] != 0x8b || header[2] != 8 ||
        (header[3] & 0xe0) != 0)
        return EFI_LOAD_ERROR;

    return EFI_SUCCESS;
}

EFI_STATUS EFIAPI efi_main(EFI_HANDLE image_handle,
                           EFI_SYSTEM_TABLE *system_table) {
    EFI_STATUS status;
    EFI_LOADED_IMAGE *loaded_image = NULL;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *filesystem = NULL;
    EFI_FILE_HANDLE root = NULL;
    EFI_HANDLE kernel_handle = NULL;
    EFI_LOADED_IMAGE *kernel_image = NULL;
    VOID *kernel_buffer = NULL;
    UINTN kernel_size = 0;
    

    InitializeLib(image_handle, system_table);
    Print(L"mBoot 0.1 - Mintosh UEFI Bootloader\r\n");
    Print(L"Locating Mintosh boot volume...\r\n");

    status = uefi_call_wrapper(BS->HandleProtocol, 3, image_handle,
                               &LoadedImageProtocol, (VOID **)&loaded_image);
    if (EFI_ERROR(status) || loaded_image == NULL ||
        loaded_image->DeviceHandle == NULL) {
        Print(L"mBoot: cannot identify the boot device.\r\n");
        return EFI_NOT_FOUND;
    }

    status = uefi_call_wrapper(BS->HandleProtocol, 3,
                               loaded_image->DeviceHandle,
                               &FileSystemProtocol, (VOID **)&filesystem);
    if (EFI_ERROR(status) || filesystem == NULL) {
        Print(L"mBoot: boot device has no EFI filesystem.\r\n");
        return EFI_UNSUPPORTED;
    }

    status = uefi_call_wrapper(filesystem->OpenVolume, 2, filesystem, &root);
    if (EFI_ERROR(status) || root == NULL) {
        Print(L"mBoot: cannot open the EFI boot volume.\r\n");
        return status;
    }

    Print(L"Loading \\boot\\vmlinuz...\r\n");
    status = read_file(root, MBOOT_KERNEL_PATH, &kernel_buffer, &kernel_size);
    if (EFI_ERROR(status)) {
        Print(L"mBoot: kernel missing, unreadable, or invalid.\r\n");
        uefi_call_wrapper(root->Close, 1, root);
        return status;
    }

    Print(L"Validating compressed initramfs...\r\n");
    status = validate_gzip_file(root, L"\\\\boot\\\\initramfs.cpio.gz");
    if (EFI_ERROR(status)) {
        Print(L"mBoot: initramfs is missing, oversized, or has an invalid gzip header.\r\n");
        FreePool(kernel_buffer);
        uefi_call_wrapper(root->Close, 1, root);
        return status;
    }
    uefi_call_wrapper(root->Close, 1, root);

    Print(L"Preparing Linux EFI stub...\r\n");
    status = uefi_call_wrapper(BS->LoadImage, 6, FALSE, image_handle,
                               loaded_image->DeviceHandle, kernel_buffer,
                               kernel_size, &kernel_handle);
    FreePool(kernel_buffer);
    if (EFI_ERROR(status) || kernel_handle == NULL) {
        Print(L"mBoot: firmware rejected the kernel image.\r\n");
        return status;
    }

    status = uefi_call_wrapper(BS->HandleProtocol, 3, kernel_handle,
                               &LoadedImageProtocol, (VOID **)&kernel_image);
    if (EFI_ERROR(status) || kernel_image == NULL) {
        Print(L"mBoot: cannot configure the loaded kernel.\r\n");
        uefi_call_wrapper(BS->UnloadImage, 1, kernel_handle);
        return status;
    }

    kernel_image->LoadOptions = AllocatePool(sizeof(mboot_kernel_options));
    if (kernel_image->LoadOptions == NULL) {
        uefi_call_wrapper(BS->UnloadImage, 1, kernel_handle);
        return EFI_OUT_OF_RESOURCES;
    }
    CopyMem(kernel_image->LoadOptions, mboot_kernel_options, sizeof(mboot_kernel_options));
    kernel_image->LoadOptionsSize = sizeof(mboot_kernel_options);

    Print(L"Starting Mintosh Linux kernel...\r\n");
    status = uefi_call_wrapper(BS->StartImage, 3, kernel_handle, NULL, NULL);
    Print(L"mBoot: kernel returned control (");
    Print(L"%r", status);
    Print(L"). Halting boot.\r\n");
    uefi_call_wrapper(BS->UnloadImage, 1, kernel_handle);
    return EFI_LOAD_ERROR;
}
