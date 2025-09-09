/**
 * @file pbkdf2.c
 * @brief PBKDF2 (Password-Based Key Derivation Function 2) Implementation for CHARM
 *
 * RFC 2898 compliant implementation with HMAC-SHA256 and CHARM integration.
 */

#include "pbkdf2.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <openssl/evp.h>

/* Secure memory functions */
static void secure_memzero(void* ptr, size_t len) {
    volatile unsigned char* vptr = (volatile unsigned char*)ptr;
    while (len--) {
        *vptr++ = 0;
    }
}

static int constant_time_compare(const uint8_t* a, const uint8_t* b, size_t length) {
    uint8_t result = 0;
    for (size_t i = 0; i < length; i++) {
        result |= a[i] ^ b[i];
    }
    return result == 0;
}

/* Initialize PBKDF2 with default secure configuration */
pbkdf2_result_t pbkdf2_init_default_config(pbkdf2_config_t* config) {
    if (!config) {
        return PBKDF2_ERROR_INVALID_PARAMS;
    }

    config->iterations = PBKDF2_DEFAULT_ITERATIONS;
    config->salt_length = PBKDF2_DEFAULT_SALT_LENGTH;
    config->derived_key_length = 32; /* Default to 256-bit key */
    config->use_charm_entropy = 1;
    config->secure_cleanup = 1;

    return PBKDF2_SUCCESS;
}

/* Initialize PBKDF2 with custom configuration */
pbkdf2_result_t pbkdf2_init(const pbkdf2_config_t* config, pbkdf2_context_t* ctx) {
    if (!config || !ctx) {
        return PBKDF2_ERROR_INVALID_PARAMS;
    }

    /* Validate configuration parameters */
    if (config->iterations < PBKDF2_MIN_ITERATIONS) {
        return PBKDF2_ERROR_INSUFFICIENT_ITERATIONS;
    }

    if (config->salt_length < PBKDF2_MIN_SALT_LENGTH) {
        return PBKDF2_ERROR_SALT_TOO_SHORT;
    }

    if (config->derived_key_length > PBKDF2_MAX_KEY_LENGTH || config->derived_key_length == 0) {
        return PBKDF2_ERROR_KEY_TOO_LONG;
    }

    /* Initialize context */
    memset(ctx, 0, sizeof(pbkdf2_context_t));
    ctx->config = *config;

    /* Allocate salt buffer */
    ctx->salt = malloc(config->salt_length);
    if (!ctx->salt) {
        return PBKDF2_ERROR_MEMORY;
    }

    /* Allocate derived key buffer */
    ctx->derived_key = malloc(config->derived_key_length);
    if (!ctx->derived_key) {
        free(ctx->salt);
        ctx->salt = NULL;
        return PBKDF2_ERROR_MEMORY;
    }

    ctx->initialized = 1;
    return PBKDF2_SUCCESS;
}

/* Generate cryptographically secure salt */
pbkdf2_result_t pbkdf2_generate_salt(uint8_t* salt, size_t salt_length, uint8_t use_charm_entropy) {
    if (!salt || salt_length < PBKDF2_MIN_SALT_LENGTH) {
        return PBKDF2_ERROR_INVALID_PARAMS;
    }

    /* Use OpenSSL's secure random number generator */
    if (RAND_bytes(salt, salt_length) != 1) {
        return PBKDF2_ERROR_CRYPTO;
    }

    /* Optionally enhance with CHARM entropy if available */
    if (use_charm_entropy) {
        /* For now, we add some additional entropy mixing */
        /* In a full CHARM integration, this would use the entropy subsystem */
        time_t now = time(NULL);
        for (size_t i = 0; i < salt_length && i < sizeof(now); i++) {
            salt[i] ^= ((uint8_t*)&now)[i];
        }
    }

    return PBKDF2_SUCCESS;
}

/* Core PBKDF2 implementation using HMAC-SHA256 */
pbkdf2_result_t pbkdf2_derive_key(
    const uint8_t* password, size_t password_length,
    const uint8_t* salt, size_t salt_length,
    uint32_t iterations,
    uint8_t* derived_key, size_t key_length
) {
    if (!password || !salt || !derived_key || 
        password_length == 0 || salt_length < PBKDF2_MIN_SALT_LENGTH || 
        key_length == 0 || iterations < PBKDF2_MIN_ITERATIONS) {
        return PBKDF2_ERROR_INVALID_PARAMS;
    }

    const size_t hash_length = SHA256_DIGEST_LENGTH;
    const size_t blocks_needed = (key_length + hash_length - 1) / hash_length;
    
    uint8_t* temp_key = malloc(blocks_needed * hash_length);
    if (!temp_key) {
        return PBKDF2_ERROR_MEMORY;
    }

    /* PBKDF2 algorithm implementation */
    for (size_t block = 0; block < blocks_needed; block++) {
        uint8_t u[SHA256_DIGEST_LENGTH];
        uint8_t t[SHA256_DIGEST_LENGTH];
        
        /* Create salt + block number */
        size_t salt_block_len = salt_length + 4;
        uint8_t* salt_block = malloc(salt_block_len);
        if (!salt_block) {
            free(temp_key);
            return PBKDF2_ERROR_MEMORY;
        }
        
        memcpy(salt_block, salt, salt_length);
        uint32_t block_num = block + 1;
        salt_block[salt_length] = (block_num >> 24) & 0xFF;
        salt_block[salt_length + 1] = (block_num >> 16) & 0xFF;
        salt_block[salt_length + 2] = (block_num >> 8) & 0xFF;
        salt_block[salt_length + 3] = block_num & 0xFF;

        /* First iteration: U_1 = HMAC(password, salt || i) */
        unsigned int hmac_len = 0;
        if (!HMAC(EVP_sha256(), password, password_length, 
                  salt_block, salt_block_len, u, &hmac_len)) {
            free(salt_block);
            free(temp_key);
            return PBKDF2_ERROR_CRYPTO;
        }
        
        memcpy(t, u, hash_length);

        /* Remaining iterations: U_n = HMAC(password, U_{n-1}) */
        for (uint32_t iter = 1; iter < iterations; iter++) {
            if (!HMAC(EVP_sha256(), password, password_length, 
                      u, hash_length, u, &hmac_len)) {
                free(salt_block);
                free(temp_key);
                return PBKDF2_ERROR_CRYPTO;
            }
            
            /* XOR with previous result */
            for (size_t j = 0; j < hash_length; j++) {
                t[j] ^= u[j];
            }
        }

        /* Copy block result to output */
        memcpy(temp_key + block * hash_length, t, hash_length);
        
        /* Secure cleanup */
        secure_memzero(u, sizeof(u));
        secure_memzero(t, sizeof(t));
        secure_memzero(salt_block, salt_block_len);
        free(salt_block);
    }

    /* Copy desired key length and cleanup */
    memcpy(derived_key, temp_key, key_length);
    secure_memzero(temp_key, blocks_needed * hash_length);
    free(temp_key);

    return PBKDF2_SUCCESS;
}

/* Complete PBKDF2 operation with context */
pbkdf2_result_t pbkdf2_derive_with_context(
    pbkdf2_context_t* ctx,
    const uint8_t* password, size_t password_length
) {
    if (!ctx || !ctx->initialized || !password) {
        return PBKDF2_ERROR_INVALID_PARAMS;
    }

    /* Generate salt if not already done */
    pbkdf2_result_t result = pbkdf2_generate_salt(
        ctx->salt, ctx->config.salt_length, ctx->config.use_charm_entropy
    );
    if (result != PBKDF2_SUCCESS) {
        return result;
    }

    /* Derive key */
    result = pbkdf2_derive_key(
        password, password_length,
        ctx->salt, ctx->config.salt_length,
        ctx->config.iterations,
        ctx->derived_key, ctx->config.derived_key_length
    );

    ctx->password_length = password_length;
    return result;
}

/* Verify password against stored key */
pbkdf2_result_t pbkdf2_verify_password(
    const uint8_t* password, size_t password_length,
    const uint8_t* salt, size_t salt_length,
    uint32_t iterations,
    const uint8_t* stored_key, size_t key_length
) {
    if (!password || !salt || !stored_key) {
        return PBKDF2_ERROR_INVALID_PARAMS;
    }

    uint8_t* derived_key = malloc(key_length);
    if (!derived_key) {
        return PBKDF2_ERROR_MEMORY;
    }

    pbkdf2_result_t result = pbkdf2_derive_key(
        password, password_length,
        salt, salt_length,
        iterations,
        derived_key, key_length
    );

    if (result == PBKDF2_SUCCESS) {
        if (!constant_time_compare(derived_key, stored_key, key_length)) {
            result = PBKDF2_ERROR_CRYPTO; /* Password mismatch */
        }
    }

    secure_memzero(derived_key, key_length);
    free(derived_key);
    return result;
}

/* Get recommended iteration count for current system */
uint32_t pbkdf2_get_recommended_iterations(void) {
    const uint8_t test_password[] = "test_password";
    const uint8_t test_salt[] = "test_salt_16_bytes_long!";
    uint8_t test_key[32];
    
    /* Start with a baseline and measure */
    uint32_t iterations = 10000;
    clock_t start = clock();
    
    pbkdf2_derive_key(
        test_password, sizeof(test_password) - 1,
        test_salt, sizeof(test_salt) - 1,
        iterations,
        test_key, sizeof(test_key)
    );
    
    clock_t end = clock();
    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    /* Target 100ms, scale iterations accordingly */
    if (time_taken > 0) {
        double target_time = 0.1; /* 100ms */
        iterations = (uint32_t)(iterations * (target_time / time_taken));
        
        /* Ensure minimum security level */
        if (iterations < PBKDF2_MIN_ITERATIONS) {
            iterations = PBKDF2_MIN_ITERATIONS;
        }
    } else {
        iterations = PBKDF2_DEFAULT_ITERATIONS;
    }
    
    secure_memzero(test_key, sizeof(test_key));
    return iterations;
}

/* Convert PBKDF2 result to string */
const char* pbkdf2_result_to_string(pbkdf2_result_t result) {
    switch (result) {
        case PBKDF2_SUCCESS:
            return "Success";
        case PBKDF2_ERROR_INVALID_PARAMS:
            return "Invalid parameters";
        case PBKDF2_ERROR_INSUFFICIENT_ITERATIONS:
            return "Insufficient iterations (minimum 10,000)";
        case PBKDF2_ERROR_SALT_TOO_SHORT:
            return "Salt too short (minimum 16 bytes)";
        case PBKDF2_ERROR_KEY_TOO_LONG:
            return "Key length too long (maximum 1024 bytes)";
        case PBKDF2_ERROR_MEMORY:
            return "Memory allocation failed";
        case PBKDF2_ERROR_CRYPTO:
            return "Cryptographic operation failed";
        default:
            return "Unknown error";
    }
}

/* Securely clear PBKDF2 context */
void pbkdf2_cleanup(pbkdf2_context_t* ctx) {
    if (!ctx) {
        return;
    }

    if (ctx->salt) {
        secure_memzero(ctx->salt, ctx->config.salt_length);
        free(ctx->salt);
        ctx->salt = NULL;
    }

    if (ctx->derived_key) {
        secure_memzero(ctx->derived_key, ctx->config.derived_key_length);
        free(ctx->derived_key);
        ctx->derived_key = NULL;
    }

    secure_memzero(ctx, sizeof(pbkdf2_context_t));
}

/* Print PBKDF2 configuration information */
void pbkdf2_print_config(const pbkdf2_config_t* config) {
    if (!config) {
        printf("PBKDF2 Configuration: (null)\n");
        return;
    }

    printf("PBKDF2 Configuration:\n");
    printf("  Iterations: %u\n", config->iterations);
    printf("  Salt length: %zu bytes\n", config->salt_length);
    printf("  Derived key length: %zu bytes\n", config->derived_key_length);
    printf("  CHARM entropy: %s\n", config->use_charm_entropy ? "enabled" : "disabled");
    printf("  Secure cleanup: %s\n", config->secure_cleanup ? "enabled" : "disabled");
}