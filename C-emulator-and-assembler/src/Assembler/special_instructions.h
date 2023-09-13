#include "assembler_utilities.h"

#define NO_OPERATION "nop"
#define DIRECTIVE ".int"
#define nop 0xD503201F

extern int32_t no_operation(int argc, char **argv) ;
extern int32_t implement_directive(int argc, char **argv);