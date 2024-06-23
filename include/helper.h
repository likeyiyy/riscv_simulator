#ifndef RISC_SIMULATOR_HELPER_H
#define RISC_SIMULATOR_HELPER_H

#include <stdlib.h>

void print_usage(const char *program_name);

int parse_arguments(int argc, char *argv[], const char **input_file, size_t *load_address, size_t *end_address);

#endif // RISC_SIMULATOR_HELPER_H