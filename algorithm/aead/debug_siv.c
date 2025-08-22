#include "../include/charm_aead.h"
#include <stdio.h>
#include <string.h>

// Test SIV generation directly
extern charm_aead_status_t charm_generate_siv(
    const uint8_t key[32], const uint8_t* aad, size_t aad_len,
    const uint8_t* plaintext, size_t plaintext_len, uint8_t siv[16]
);

static void print_hex(const char* label, const uint8_t* data, size_t len) {
    printf("%s: ", label);
    for (size_t i = 0; i < len; i++) {
        printf("%02x", data[i]);
    }
    printf("\n");
}

int main() {
    uint8_t key[32] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
    };
    
    const char* aad_str = "Common AAD";
    const uint8_t* aad = (const uint8_t*)aad_str;
    size_t aad_len = strlen(aad_str);
    
    const char* msg1 = "Message 1";
    const char* msg2 = "Message 2";
    const uint8_t* plaintext1 = (const uint8_t*)msg1;
    const uint8_t* plaintext2 = (const uint8_t*)msg2;
    size_t len1 = strlen(msg1);
    size_t len2 = strlen(msg2);
    
    uint8_t siv1[16], siv2[16];
    
    charm_generate_siv(key, aad, aad_len, plaintext1, len1, siv1);
    charm_generate_siv(key, aad, aad_len, plaintext2, len2, siv2);
    
    print_hex("Plaintext1", plaintext1, len1);
    print_hex("Plaintext2", plaintext2, len2);
    print_hex("SIV1", siv1, 16);
    print_hex("SIV2", siv2, 16);
    
    if (memcmp(siv1, siv2, 16) == 0) {
        printf("ERROR: SIVs are identical!\n");
        return 1;
    } else {
        printf("SUCCESS: SIVs are different\n");
        return 0;
    }
}