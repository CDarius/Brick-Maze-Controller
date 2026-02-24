#pragma once

#include <Arduino.h>
#include <SerialCommand.hpp>

/**
 * SerialCommandReader class for parsing parameters from a SerialCommand.
 * Parameters in the values field are separated by '##'.
 * Each get call reads the next parameter sequentially.
 */
class SerialCommandReader {
public:
    /**
     * Constructor
     * @param cmd The SerialCommand to parse
     */
    SerialCommandReader(const SerialCommand& cmd) 
        : values(cmd.values), currentIndex(0) {}
    
    /**
     * Read and parse the next parameter as a 32-bit signed integer
     * @param value Output parameter to store the parsed integer
     * @return true if successfully parsed, false otherwise
     */
    bool getInt32(int32_t& value);
    
    /**
     * Read and parse the next parameter as a 32-bit unsigned integer
     * @param value Output parameter to store the parsed unsigned integer
     * @return true if successfully parsed, false otherwise
     */
    bool getUInt32(uint32_t& value);
    
    /**
     * Read and parse the next parameter as a float
     * @param value Output parameter to store the parsed float
     * @return true if successfully parsed, false otherwise
     */
    bool getFloat(float& value);
    
    /**
     * Read and parse the next parameter as a boolean
     * @param value Output parameter to store the parsed boolean
     * @return true if successfully parsed, false otherwise
     */
    bool getBool(bool& value);

private:
    String values;
    int currentIndex;
    
    /**
     * Get the next token from the values string
     * @return The next token or empty string if no more tokens
     */
    inline String getNextToken() {
        if (currentIndex >= values.length()) {
            return "";
        }
        
        int separatorIndex = values.indexOf(SERIAL_COMMAND_PARAMS_DELIMITER, currentIndex);
        String token;
        
        if (separatorIndex == -1) {
            // Last token
            token = values.substring(currentIndex);
            currentIndex = values.length();
        } else {
            // Extract token between current position and separator
            token = values.substring(currentIndex, separatorIndex);
            currentIndex = separatorIndex + strlen(SERIAL_COMMAND_PARAMS_DELIMITER); // Move past the delimiter
        }
        
        token.trim(); // Remove leading/trailing whitespace
        return token;
    }
};
