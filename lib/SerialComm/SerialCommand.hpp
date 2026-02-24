#pragma once

#include <Arduino.h>

#define SERIAL_COMMAND_PARAM_SEPARATOR ':'
#define SERIAL_COMMAND_PARAMS_DELIMITER "##"

/**
 * SerialCommand struct is used to represent a parsed command received from the serial interface. It contains:
 * - command: The main command string (e.g., "MOVE", "SET", etc.)
 * - values: The associated values or parameters for the command (e.g., "10,20")
 * - isValid: A boolean flag indicating whether the command was successfully parsed and is valid
 */
struct SerialCommand {
    String command;
    String values;
    bool isValid;
};
