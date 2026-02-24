#include "SerialComm.hpp"

SerialCommand SerialComm::readCommands() {
    SerialCommand cmd;
    cmd.isValid = false;
    cmd.command = "";
    cmd.values = "";

    // Read all available bytes at once
    int available = serial.available();
    if (available > 0) {
        char buffer[available + 1];
        size_t bytesRead = serial.readBytes(buffer, available);
        buffer[bytesRead] = '\0';  // Null terminate
        
        receiveBuffer += String(buffer);  // Append to buffer
    }

    if (receiveBuffer.length() > 0) {
        // Search for complete line 
        int newlineIndex = receiveBuffer.indexOf('\n');
        if (newlineIndex != -1) {
            String line = receiveBuffer.substring(0, newlineIndex);            
            receiveBuffer = receiveBuffer.substring(newlineIndex + 1); // Remove processed line

            // Find the colon separator
            int colonIndex = line.indexOf(SERIAL_COMMAND_PARAM_SEPARATOR);
            if (colonIndex > 0 && colonIndex < line.length() - 1) {
                cmd.command = line.substring(0, colonIndex);
                cmd.values = line.substring(colonIndex + 1);
                cmd.isValid = true;
            } else {
                // Invalid command format
                Serial.println("Invalid command format received: " + line);
            }
        }
    }

    // No complete line found
    return cmd;
}
