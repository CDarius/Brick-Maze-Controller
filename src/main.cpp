#include <Arduino.h>
#include <Wire.h>

#include "PinDefinitions.h"
#include <MPU6886.hpp>
#include <Controller.hpp>
#include <SerialComm.hpp>
#include <SerialCommandReader.hpp>
#include <MatrixDisplay.hpp>
#include <HMI.hpp>

MatrixDisplay display(MATRIX_LED_PIN);
HMI hmi(display);
MPU6886 imu(Wire);
Controller controller(imu, MATRIX_BUTTON_PIN);
SerialComm serailComm(Serial1);

void showInitFailed(const char* displayMessage, const char* serialMessage) {
    //display.clear();
    //String displayMessageStr(displayMessage);
    //displayMessageStr.toUpperCase();
    //display.drawString(0,0, displayMessageStr, COLOR_RED, FONT_4x6);
    //display.show();
    while (true)
    {
        Serial.println(serialMessage);
        delay(500);
    }
}

void setup() {
    // Start serial communication first for debugging purposes.
    Serial.begin(115200);
    
    // Initialize serial comunication with the remote controller
    Serial1.begin (115200, SERIAL_8N1, REMOTE_CONTROLLER_UART_RX, REMOTE_CONTROLLER_UART_TX, false);
    Serial1.setTimeout(0);

    // Initialize random seed with noise from an unconnected ADC pin for better randomness in the game
    randomSeed(analogRead(0));

    // Initialize the display
    display.begin();
    display.setBrightness(40);
    display.clear();
    display.show();

    // Initialize internal I2C. The MPU6886 on the M5Atom is on this bus.
    // The MPU6886 library will use the default 'Wire' object.
    Wire.begin(INTERNAL_I2C_SDA, INTERNAL_I2C_SCL, 400000); // 400kHz for faster communication
    delay(500);

    // Initialize MPU6886 and controller
    if (!imu.begin()) {
        showInitFailed("MPU6886 INIT FAILED", "Failed to initialize MPU6886 IMU sensor");
    }
    controller.begin();

    // Create display_task on core 1
    xTaskCreatePinnedToCore(
        [](void* param) { hmi.updateLoop(); }, // Task function
        "DisplayTask",                      // Name
        4096,                               // Stack size
        nullptr,                            // Parameter
        1,                                  // Priority
        nullptr,                            // Task handle
        1                                   // Core 1
    );
    
    Serial.println("Initialization successful. Starting main loop...");
}
    
void readIncomingMessages();

float accX, accY;

void loop() {
    // Send XY angles and button state to the host if the controller is enabled and it's time for an update
    if (controller.getXY(accX, accY)) {
        serailComm.sendXYangles(accX, accY, false);
        bool buttonPressed = controller.isButtonPressed();
        serailComm.sendXYangles(accX, accY, buttonPressed);

        // Send IMU data also to HMI
        float gz, gy, gx;
        imu.getGyro(&gx, &gy, &gz);
        hmi.updateIMUData(accX, accY, 0.0f, gx, gy, gz);
    }

    readIncomingMessages();

    delay(1);
}

void readIncomingMessages() {
    // Read and process incoming serial commands
    SerialCommand cmd = serailComm.readCommands();
    if (cmd.isValid) {
        SerialCommandReader reader(cmd);
        String command = cmd.command;
        if (command == "SET_CTRL_PARAMS") {
            float maxAcc;
            uint32_t updateRate;
            if (reader.getFloat(maxAcc) && reader.getUInt32(updateRate)) {
                controller.setMaxAccValue(maxAcc);
                controller.setUpdateRate(updateRate);
                Serial.printf("Max acceleration value set to %.3f\n", maxAcc);
                Serial.printf("Update rate set to %u ms\n", updateRate);
            } else {
                Serial.println("Invalid parameters for SET_CTRL_PARAMS command");
            }
        } else if (command == "ENAB_CTRL") {
            bool enable;
            if (reader.getBool(enable)) {
                controller.setIsEnabled(enable);
                Serial.printf("Controller %s\n", enable ? "enabled" : "disabled");
            } else {
                Serial.println("Invalid parameter for ENAB_CTRL command");
            }
        } else {
            Serial.println("Unknown command received: " + command);
        }
    }
}