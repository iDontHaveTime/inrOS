#include "bootio.h"

#include "bootdef.h"

VOID EFIAPI clearscreen(VOID) {
    SystemTable->ConOut->ClearScreen(SystemTable->ConOut);
}

VOID EFIAPI print(CHAR16* str) {
    SystemTable->ConOut->OutputString(SystemTable->ConOut, str);
}

VOID EFIAPI newline(VOID) {
    SystemTable->ConOut->OutputString(SystemTable->ConOut, L"\r\n");
}

VOID EFIAPI println(CHAR16* str) {
    print(str);
    newline();
}

static const CHAR16* numberToRepresentation = L"0123456789ABCDEF";

VOID EFIAPI printh8(uint8_t i) {
    CHAR16 buffer[3] = {L'0', L'0', 0};
    buffer[0] = numberToRepresentation[i >> 4];
    buffer[1] = numberToRepresentation[i & 0xF];
    print(buffer);
}

VOID EFIAPI printh16(uint16_t i) {
    CHAR16 buffer[5] = {L'0', L'0', L'0', L'0', 0};
    buffer[0] = numberToRepresentation[(i >> 12)];
    buffer[1] = numberToRepresentation[(i >> 8) & 0xF];
    buffer[2] = numberToRepresentation[(i >> 4) & 0xF];
    buffer[3] = numberToRepresentation[i & 0xF];
    print(buffer);
}

VOID EFIAPI printh32(uint32_t i) {
    CHAR16 buffer[9] = {L'0', L'0', L'0', L'0', L'0', L'0', L'0', L'0', 0};
    buffer[0] = numberToRepresentation[(i >> 28)];
    buffer[1] = numberToRepresentation[(i >> 24) & 0xF];
    buffer[2] = numberToRepresentation[(i >> 20) & 0xF];
    buffer[3] = numberToRepresentation[(i >> 16) & 0xF];
    buffer[4] = numberToRepresentation[(i >> 12) & 0xF];
    buffer[5] = numberToRepresentation[(i >> 8) & 0xF];
    buffer[6] = numberToRepresentation[(i >> 4) & 0xF];
    buffer[7] = numberToRepresentation[i & 0xF];
    print(buffer);
}

VOID EFIAPI printh64(uint64_t i) {
    CHAR16 buffer[17] = {L'0', L'0', L'0', L'0', L'0', L'0', L'0', L'0', L'0',
                         L'0', L'0', L'0', L'0', L'0', L'0', L'0', 0};
    buffer[0] = numberToRepresentation[(i >> 60)];
    buffer[1] = numberToRepresentation[(i >> 56) & 0xF];
    buffer[2] = numberToRepresentation[(i >> 52) & 0xF];
    buffer[3] = numberToRepresentation[(i >> 48) & 0xF];
    buffer[4] = numberToRepresentation[(i >> 44) & 0xF];
    buffer[5] = numberToRepresentation[(i >> 40) & 0xF];
    buffer[6] = numberToRepresentation[(i >> 36) & 0xF];
    buffer[7] = numberToRepresentation[(i >> 32) & 0xF];
    buffer[8] = numberToRepresentation[(i >> 28) & 0xF];
    buffer[9] = numberToRepresentation[(i >> 24) & 0xF];
    buffer[10] = numberToRepresentation[(i >> 20) & 0xF];
    buffer[11] = numberToRepresentation[(i >> 16) & 0xF];
    buffer[12] = numberToRepresentation[(i >> 12) & 0xF];
    buffer[13] = numberToRepresentation[(i >> 8) & 0xF];
    buffer[14] = numberToRepresentation[(i >> 4) & 0xF];
    buffer[15] = numberToRepresentation[i & 0xF];
    print(buffer);
}

EFI_FILE_HANDLE EFIAPI getThisVolume(VOID) {
    EFI_LOADED_IMAGE* loadedImage = NULL;
    EFI_GUID lipGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* ioVol;
    EFI_GUID fsGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    EFI_FILE_HANDLE volume;

    SystemTable->BootServices->HandleProtocol(ImageHandle, &lipGuid,
                                              (void**)&loadedImage);
    SystemTable->BootServices->HandleProtocol(loadedImage->DeviceHandle,
                                              &fsGuid, (void**)&ioVol);
    ioVol->OpenVolume(ioVol, &volume);
    return volume;
}

EFI_FILE_HANDLE EFIAPI openFileRO(EFI_FILE_HANDLE volume, CHAR16* fileName) {
    EFI_FILE_HANDLE handle;
    volume->Open(volume, &handle, fileName, EFI_FILE_MODE_READ,
                 EFI_FILE_READ_ONLY | EFI_FILE_HIDDEN | EFI_FILE_SYSTEM);
    return handle;
}

VOID EFIAPI closeFile(EFI_FILE_HANDLE file) {
    file->Close(file);
}

UINT64 EFIAPI getFileSize(EFI_FILE_HANDLE file) {
    EFI_GUID finfoGuid = EFI_FILE_INFO_ID;
    UINTN size = 0;

    EFI_STATUS status = file->GetInfo(file, &finfoGuid, &size, NULL);
    if(status != EFI_BUFFER_TOO_SMALL) {
        println(L"[ERROR] Couldn't get file info");
        return -1;
    }

    EFI_FILE_INFO* buffer = NULL;
    status = SystemTable->BootServices->AllocatePool(EfiLoaderData, size,
                                                     (void**)&buffer);
    if(status != EFI_SUCCESS) {
        println(L"[ERROR] Failed to allocate memory for file info");
        return -1;
    }

    status = file->GetInfo(file, &finfoGuid, &size, buffer);
    if(status != EFI_SUCCESS) {
        println(L"[ERROR] Failed to get file info after allocating the buffer");
        SystemTable->BootServices->FreePool(buffer);
        return -1;
    }

    UINT64 fileSize = buffer->FileSize;

    SystemTable->BootServices->FreePool(buffer);

    return fileSize;
}

char* EFIAPI loadFileIntoMemory(EFI_FILE_HANDLE file, UINTN fileSize) {
    char* buffer = NULL;
    EFI_STATUS status = SystemTable->BootServices->AllocatePool(
        EfiLoaderData, fileSize, (void**)&buffer);
    if(status != EFI_SUCCESS) {
        println(L"[ERROR] Failed to load file into memory");
        return NULL;
    }

    UINTN bytesRead = fileSize;
    file->Read(file, &bytesRead, buffer);
    if(bytesRead != fileSize) {
        println(L"[ERROR] Couldn't load the whole file into memory");
        SystemTable->BootServices->FreePool(buffer);
        return NULL;
    }

    return buffer;
}
