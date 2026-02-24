#pragma once

#include <Arduino.h>
#include <SerialCommand.hpp>

/**
 * SerialComm class is responsible for handling serial communication with the host. It provides methods to send formatted
 * data (like XY angles) to the host and to read incoming commands from the serial buffer. The readCommands method is
 * non-blocking and processes complete lines of input, parsing them into command and value components.
 */
class SerialComm {
    public:
        /**
         * Constructor for SerialComm.
         * @param serial Reference to the HardwareSerial object to use for communication.
         */
        SerialComm(HardwareSerial& serial) : serial(serial), receiveBuffer("") {}

        /**
         * Sends the current XY angles and button state to the host.
         * @param x The X angle.
         * @param y The Y angle.
         * @param buttonPressed The state of the button (true if pressed, false otherwise).
         */
        void sendXYangles(float x, float y, bool buttonPressed) {
            // Format the string as "DATA:<x value>,<y value>,<button state>\n"
            serial.printf("DATA:%.3f##%.3f##%d\n", x, y, buttonPressed);
        }

        /**
         * Reads commands from the serial buffer in a non-blocking manner.
         * @return A SerialCommand struct containing the parsed command and its values.
         */
        SerialCommand readCommands();
        
    private:
        HardwareSerial& serial;
        String receiveBuffer;  // Buffer to accumulate incoming data until newline
};