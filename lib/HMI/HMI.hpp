#pragma once

#include <Arduino.h>
#include <MatrixDisplay.hpp>
#include <CancelToken.hpp>

class HMI {
public:
    enum class Mode : uint8_t {
        NO_GAME = 0,
        IN_GAME = 1,
        END_GAME = 2,
        WRITE_PLAYER_NAME = 3,
        WAITING_TO_START = 4,
    };

    HMI(MatrixDisplay& display) : display(display) {
        setMode(Mode::NO_GAME); // Start in No Game mode by default
    };

    /**
     * Set the current mode of the HMI. This will change the display behavior based on the selected mode.
     * @param nowMode The new mode to set.
     */
    void setMode(Mode nowMode);

    /**
     * Call this function in a task to continuously update the display based on the current mode and controller state. 
     * This function will block, so it should be run in a separate task from the main loop.
     */
    void updateLoop();

    /**
     * Update the latest IMU data for potential display in the HMI. This can be used to show tilt or motion information on the display if desired.
     * @param ax Acceleration in the X axis
     * @param ay Acceleration in the Y axis
     * @param az Acceleration in the Z axis
     * @param gx Gyro rotation around the X axis
     * @param gy Gyro rotation around the Y axis
     * @param gz Gyro rotation around the Z axis
     */
    void updateIMUData(float ax, float ay, float az, float gx, float gy, float gz);

private:
    MatrixDisplay& display;

    Mode currentMode;
    CancelToken* cancelToken;

    float ax, ay, az; // Latest acceleration data
    float gx, gy, gz; // Latest gyro data

    bool isDizzy = false; // Controller has been shaken hard
    unsigned long dizzyEndTime = 0;

    // 1980s arcade palette
    RgbColor neonCyan = RgbColor(0, 200, 255);   // Main movement color
    RgbColor neonMagenta = RgbColor(255, 0, 150); // Color for center/crosshair
    RgbColor dimCyan = neonCyan.Dim(64); // Trail/glow effect

    void noGameUpdateLoop();
    void inGameUpdateLoop();
    void endGameUpdateLoop();
    void writePlayerNameUpdateLoop();
    void waitingToStartUpdateLoop();

    /**
     * Display a rainbow stripes animation that scrolls across the display. The colors will shift 
     * over time to create a dynamic effect.
     */
    void rainbowStripesLoop(uint32_t maxTimeMs, CancelToken& cancelToken);

    /**
     * Display a simple "snake" animation that moves around the display. The snake will change direction 
     * when it hits the edges of the display. The colors of the snake will cycle through the hue spectrum.
     */
    void snakeLoop(uint32_t maxTimeMs, CancelToken& cancelToken);

    /**
     * Display a simple eye animation that reacts to the tilt of the controller. The eyes will look in the direction 
     * of the tilt, and occasionally blink.
     */
    void eyesLoop(uint32_t maxTimeMs, CancelToken& cancelToken);

    /**
     * Display an arrow pointing in the direction indicated by the controller's tilt. 
     * The direction is determined by the latest IMU data and will update in real-time as the controller is tilted. 
     * The arrow will point in one of 8 directions (N, NE, E, SE, S, SW, W, NW) based on the tilt angle.
     */
    void displayPointingDirection(uint8_t& direction);

    /**
     * Display a dizzy face animation with rotating eyes, used when the controller detects 
     * a shake. The animation will last for a short duration to indicate the dizzy state.
     */
    void displayDizzyEyes();
};