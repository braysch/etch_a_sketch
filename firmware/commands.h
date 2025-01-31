#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdint.h>

// Command types enum
typedef enum {
    COMMAND_POINT,
    COMMAND_HOME,
    COMMAND_DRIVE,
    COMMAND_INVALID
} command_type_t;

#endif // COMMANDS_H
