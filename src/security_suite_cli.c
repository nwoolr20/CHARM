/**
 * @file security_suite_cli.c
 * @brief Command line interface for the CHARM Security Suite
 *
 * This CLI provides convenient access to core CHARM Security Suite
 * functionality including status reporting, capability inspection,
 * health checks, incident reporting and SBOM generation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <getopt.h>
#include <stdint.h>

#include "charm_security_suite.h"

/* Print usage information */
static void print_help(void) {
    printf("CHARM Security Suite CLI\n\n");
    printf("Usage: charm_security_suite [OPTIONS]\n\n");
    printf("Options:\n");
    printf("  --status                Display current security suite status\n");
    printf("  --capabilities          List enabled security capabilities\n");
    printf("  --health                Run comprehensive health check\n");
    printf("  --sbom FILE             Generate SBOM in JSON format and write to FILE\n");
    printf("  --report TYPE:DESC      Report security incident with type and description\n");
    printf("  --version               Show security suite version\n");
    printf("  --help                  Show this help message\n\n");
    printf("Examples:\n");
    printf("  charm_security_suite --status\n");
    printf("  charm_security_suite --capabilities --version\n");
    printf("  charm_security_suite --health\n");
    printf("  charm_security_suite --report AUTH_FAILURE:\"Invalid password\"\n");
    printf("  charm_security_suite --sbom sbom.json\n");
}

/* Map string to incident type enum */
static charm_incident_type_t parse_incident_type(const char* type) {
    if (strcmp(type, "AUTH_FAILURE") == 0) {
        return CHARM_INCIDENT_AUTHENTICATION_FAILURE;
    } else if (strcmp(type, "AUTHZ_VIOLATION") == 0) {
        return CHARM_INCIDENT_AUTHORIZATION_VIOLATION;
    } else if (strcmp(type, "CRYPTO_FAILURE") == 0) {
        return CHARM_INCIDENT_CRYPTO_FAILURE;
    } else if (strcmp(type, "KEY_COMPROMISE") == 0) {
        return CHARM_INCIDENT_KEY_COMPROMISE;
    } else if (strcmp(type, "AUDIT_TAMPERING") == 0) {
        return CHARM_INCIDENT_AUDIT_TAMPERING;
    } else if (strcmp(type, "CONFIG_VIOLATION") == 0) {
        return CHARM_INCIDENT_CONFIG_VIOLATION;
    } else if (strcmp(type, "ENTROPY_DEGRADATION") == 0) {
        return CHARM_INCIDENT_ENTROPY_DEGRADATION;
    }
    return CHARM_INCIDENT_UNKNOWN_THREAT;
}

int main(int argc, char* argv[]) {
    int opt;
    int option_index = 0;
    bool show_status = false;
    bool show_caps = false;
    bool show_health = false;
    bool show_version = false;
    char* sbom_file = NULL;
    char* report_arg = NULL;

    static struct option long_options[] = {
        {"status", no_argument, 0, 's'},
        {"capabilities", no_argument, 0, 'c'},
        {"health", no_argument, 0, 'h'},
        {"sbom", required_argument, 0, 'b'},
        {"report", required_argument, 0, 'r'},
        {"version", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'H'},
        {0, 0, 0, 0}
    };

    if (argc == 1) {
        print_help();
        return 0;
    }

    while ((opt = getopt_long(argc, argv, "schb:r:vH", long_options, &option_index)) != -1) {
        switch (opt) {
            case 's':
                show_status = true;
                break;
            case 'c':
                show_caps = true;
                break;
            case 'h':
                show_health = true;
                break;
            case 'b':
                sbom_file = optarg;
                break;
            case 'r':
                report_arg = optarg;
                break;
            case 'v':
                show_version = true;
                break;
            case 'H':
                print_help();
                return 0;
            default:
                print_help();
                return 1;
        }
    }

    if (charm_security_suite_init(NULL) != 0) {
        fprintf(stderr, "Failed to initialize CHARM Security Suite\n");
        return 1;
    }

    int exit_code = 0;

    if (show_version) {
        printf("Version: %s\n", charm_security_suite_get_version());
    }

    if (show_caps) {
        uint32_t caps;
        if (charm_security_suite_get_capabilities(&caps) == 0) {
            printf("Enabled capabilities: 0x%08X\n", caps);
        } else {
            fprintf(stderr, "Error retrieving capabilities\n");
            exit_code = 1;
        }
    }

    if (show_status) {
        charm_security_suite_status_t status;
        if (charm_security_suite_get_status(&status) == 0) {
            printf("Initialized: %s\n", status.initialized ? "yes" : "no");
            printf("Security level: %d\n", status.current_security_level);
            printf("Entropy quality: %.4f\n", status.entropy_quality);
            printf("Incidents recorded: %lu\n", (unsigned long)status.total_audit_events);
        } else {
            fprintf(stderr, "Error retrieving status\n");
            exit_code = 1;
        }
    }

    if (show_health) {
        char issues[10][256];
        size_t issue_count = 0;
        int score = charm_security_suite_health_check(issues, 10, &issue_count);
        printf("Health score: %d/100\n", score);
        for (size_t i = 0; i < issue_count; i++) {
            printf(" - %s\n", issues[i]);
        }
    }

    if (sbom_file) {
        if (charm_security_suite_generate_sbom(sbom_file, "json") == 0) {
            printf("SBOM written to %s\n", sbom_file);
        } else {
            fprintf(stderr, "Failed to generate SBOM\n");
            exit_code = 1;
        }
    }

    if (report_arg) {
        char* colon = strchr(report_arg, ':');
        if (!colon) {
            fprintf(stderr, "Invalid --report argument. Use TYPE:DESC\n");
            exit_code = 1;
        } else {
            *colon = '\0';
            const char* type_str = report_arg;
            const char* desc = colon + 1;
            charm_incident_type_t type = parse_incident_type(type_str);
            uint64_t id = 0;
            if (charm_security_suite_report_incident(type, "cli", desc, NULL, 5, &id) == 0) {
                printf("Incident reported with ID %lu\n", id);
            } else {
                fprintf(stderr, "Failed to report incident\n");
                exit_code = 1;
            }
        }
    }

    charm_security_suite_shutdown();
    return exit_code;
}
