#include "../include/charm.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static void print_hex(const char* label, const uint8_t* data, size_t len) {
    printf("%s: ", label);
    for (size_t i = 0; i < len; i++) {
        printf("%02x", data[i]);
        if (i > 0 && (i + 1) % 16 == 0) printf("\n%*s", (int)strlen(label) + 2, "");
    }
    printf("\n");
}

int main() {
    // Recreate the exact inputs from SIV generation
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
    
    // Build inputs exactly as in charm_generate_siv
    size_t input_len = 32 + 3 + 8 + aad_len + 8 + len1; // Both have same length
    uint8_t* input1 = malloc(input_len);
    uint8_t* input2 = malloc(input_len);
    
    // Input 1
    size_t offset = 0;
    memcpy(input1 + offset, key, 32); offset += 32;
    memcpy(input1 + offset, "SIV", 3); offset += 3;
    for (int i = 0; i < 8; i++) input1[offset + i] = (uint8_t)((aad_len >> (i * 8)) & 0xFF); offset += 8;
    memcpy(input1 + offset, aad, aad_len); offset += aad_len;
    for (int i = 0; i < 8; i++) input1[offset + i] = (uint8_t)((len1 >> (i * 8)) & 0xFF); offset += 8;
    memcpy(input1 + offset, plaintext1, len1);
    
    // Input 2
    offset = 0;
    memcpy(input2 + offset, key, 32); offset += 32;
    memcpy(input2 + offset, "SIV", 3); offset += 3;
    for (int i = 0; i < 8; i++) input2[offset + i] = (uint8_t)((aad_len >> (i * 8)) & 0xFF); offset += 8;
    memcpy(input2 + offset, aad, aad_len); offset += aad_len;
    for (int i = 0; i < 8; i++) input2[offset + i] = (uint8_t)((len2 >> (i * 8)) & 0xFF); offset += 8;
    memcpy(input2 + offset, plaintext2, len2);
    
    uint8_t hash1[64], hash2[64];
    
    int status1 = charm_hash(CHARM_512, input1, input_len, hash1);
    int status2 = charm_hash(CHARM_512, input2, input_len, hash2);
    
    printf("Status1: %d, Status2: %d\n", status1, status2);
    printf("Input length: %zu\n", input_len);
    
    print_hex("Input1", input1, input_len);
    print_hex("Input2", input2, input_len);
    print_hex("Hash1", hash1, 64);
    print_hex("Hash2", hash2, 64);
    
    // Compare just first 16 bytes (SIV)
    printf("SIV1: ");
    for (int i = 0; i < 16; i++) printf("%02x", hash1[i]);
    printf("\n");
    printf("SIV2: ");
    for (int i = 0; i < 16; i++) printf("%02x", hash2[i]);
    printf("\n");
    
    if (memcmp(hash1, hash2, 16) == 0) {
        printf("ERROR: SIVs are identical!\n");
        return 1;
    } else {
        printf("SUCCESS: SIVs are different\n");
        return 0;
    }
}