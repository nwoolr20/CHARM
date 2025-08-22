#include "../include/charm.h"
#include <stdio.h>
#include <string.h>

static void print_hex(const char* label, const uint8_t* data, size_t len) {
    printf("%s: ", label);
    for (size_t i = 0; i < len; i++) {
        printf("%02x", data[i]);
    }
    printf("\n");
}

int main() {
    uint8_t input1[] = "test1";
    uint8_t input2[] = "test2";
    uint8_t hash1[64], hash2[64];
    
    int status1 = charm_hash(CHARM_512, input1, 5, hash1);
    int status2 = charm_hash(CHARM_512, input2, 5, hash2);
    
    printf("Status1: %d, Status2: %d\n", status1, status2);
    
    print_hex("Input1", input1, 5);
    print_hex("Input2", input2, 5);
    print_hex("Hash1", hash1, 64);
    print_hex("Hash2", hash2, 64);
    
    if (memcmp(hash1, hash2, 64) == 0) {
        printf("ERROR: Hashes are identical!\n");
        return 1;
    } else {
        printf("SUCCESS: Hashes are different\n");
        return 0;
    }
}