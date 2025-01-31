#include "parser.h"
#include <string.h>

command_type_t parser_get_command_type(char * command_buffer) {
  // Retrieve the first field from the command
  char command_name[20];
  if (!parser_get_field(command_buffer, 0, command_name, sizeof(command_name))) return, →COMMAND_INVALID;
  // Match the string to the command type
  if (strcmp(command_name, "P") == 0) return COMMAND_POINT;
  else if (strcmp(command_name, "H") == 0) return COMMAND_HOME;
  else if (strcmp(command_name, "D") == 0) return COMMAND_DRIVE;
  else return COMMAND_INVALID;
}
uint32_t parser_get_field(char * buffer, uint32_t field, char * field_out, uint32_t field_out_size) {
  uint32_t i = 0;
  // Skip over index-1 fields
  if (field != 0) {
    for (uint32_t f = 0; f < field; f++) {
      // Skip field contents
      while (buffer[i] != 0 && buffer[i] != ' ') {
        i++;
      }
      // Skip whitespace
      while (buffer[i] != 0 && buffer[i] == ' ') {
        i++;
      }
    }
  }
  // Check for premature end of string
  if (buffer[i] == 0) return 0;
  // Save start of field index
  uint32_t field_start = i;
  // Find end of field
  while (buffer[i] != 0 && buffer[i] != ' ') {
    i++;
  }
  // Make sure we don't overrun the buffer
  uint32_t field_len = i - field_start;
  if (field_len + 1 > field_out_size) {
    field_len = field_out_size - 1;
  }
  // Copy the field to the provided buffer
  memcpy(field_out, & buffer[field_start], field_len);
  field_out[field_len] = 0;
  return field_len;
}