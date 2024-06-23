#include <stdio.h>
#include <getopt.h>
#include "memory.h"
#include "helper.h"

void print_usage(const char *program_name) {
    fprintf(stderr, "Usage: %s --rom <input file> --load_address <load address> [--end_address <end address>]\n", program_name);
}

int parse_arguments(int argc, char *argv[], const char **input_file, size_t *load_address, size_t *end_address) {
    struct option long_options[] = {
            {"rom", required_argument, 0, 'r'},
            {"load_address", required_argument, 0, 'l'},
            {"end_address", required_argument, 0, 'e'},
            {"help", no_argument, 0, 'h'},
            {0, 0, 0, 0}
    };

    int option_index = 0;
    int c;
    while ((c = getopt_long(argc, argv, "r:l:e:h", long_options, &option_index)) != -1) {
        switch (c) {
            case 'r':
                if (optarg == NULL || *optarg == '\0') {
                    fprintf(stderr, "Error: --rom requires a non-empty argument\n");
                    return 1;
                }
                *input_file = optarg;
                break;
            case 'l':
                if (optarg == NULL || *optarg == '\0') {
                    fprintf(stderr, "Error: --load_address requires a non-empty argument\n");
                    return 1;
                }
                *load_address = strtoull(optarg, NULL, 0);
                break;
            case 'e':
                if (optarg == NULL || *optarg == '\0') {
                    fprintf(stderr, "Error: --end_address requires a non-empty argument\n");
                    return 1;
                }
                *end_address = strtoull(optarg, NULL, 0);
                break;
            case 'h':
            case '?':
                return 1;
            default:
                return 1;
        }
    }

    if (*input_file == NULL || *load_address == 0) {
        return 1;
    }

    if (*end_address == 0) {
        *end_address = MEMORY_END_ADDR;
    }

    if (*load_address >= MEMORY_END_ADDR || (*end_address > MEMORY_END_ADDR) || (*load_address >= *end_address)) {
        fprintf(stderr, "Invalid address range. Load address and end address must be within memory size (0x%llx bytes), load_address: 0x%llx, end_address: 0x%llx\n", MEMORY_END_ADDR, *load_address, *end_address);
        return 1;
    }

    return 0;
}
