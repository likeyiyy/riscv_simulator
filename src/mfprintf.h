//
// Created by likeyiyy on 24-6-9.
//

#ifndef RISCSIMULATOR_MFPRINTF_H
#define RISCSIMULATOR_MFPRINTF_H

#include <stdio.h>
#include <stdarg.h>

#define FILENAME "riscv_simulator_output.log"
int mfprintf(const char *format, ...);

#endif //RISCSIMULATOR_MFPRINTF_H
