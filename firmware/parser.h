#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>

// Enum to represent the different types of commands
typedef enum {
    COMMAND_INVALID = 0,
    COMMAND_POINT,
    COMMAND_HOME,
    COMMAND_DRIVE
} command_type_t;

// Function prototypes
command_type_t parser_get_command_type(char * command_buffer);
uint32_t parser_get_field(char * buffer, uint32_t field, char * field_out, uint32_t field_out_size);

#endif // PARSER_H
