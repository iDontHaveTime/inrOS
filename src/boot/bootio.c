#include "bootio.h"

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
