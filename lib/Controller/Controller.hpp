#pragma once

#include <Arduino.h>

#include <MPU6886.hpp>

/**
 * Controller class is responsible for interfacing with the MPU6886 IMU sensor to read accelerometer data 
 * and convert it into X and Y angles. It also handles a button input. 
 * The getXY method returns the current X and Y angles based on the accelerometer readings, 
 * applying constraints and filtering as needed.
 */
class Controller {
public:
    Controller(MPU6886& imu, uint8_t buttonPin) : imu(imu), buttonPin(buttonPin) {}

    /**
     * Initializes the controller by setting up the button pin and configuring initial parameters.
     * This should be called in the setup() function of the main program.
     */
    void begin();

    /**
     * Reads the current X and Y angles based on the accelerometer data. The angles are constrained to 
     * a maximum value defined by maxAccValue. The method also implements a simple filter to prevent 
     * sudden jumps in angles when the device is tilted beyond the horizontal plane.
     */
    bool getXY(float& x, float& y);
    
    /**
     * Gets the maximum acceleration value that corresponds to the maximum angle.
     */
    float getMaxAccValue() const { return maxAccValue; }
    /**
     * Set the maximum acceleration value that corresponds to the maximum angle. This is used to limit
     * the output angles to a certain range based on the expected maximum tilt of the device. For example,
     * if the device is expected to be tilted up to 45 degrees, you might set this to around 0.7g 
     * (since sin(45°) ≈ 0.707).
     */
    void setMaxAccValue(float value) { maxAccValue = value; }

    /**
     * Check if the controller is currently enabled.
     */
    bool getIsEnabled() const { return isEnabled; }
    /**
     * Enable or disable the controller. When disabled, getXY will return false and not update the angles.
     * This can be useful to temporarily stop sending updates to the host
     */
    void setIsEnabled(bool enabled) { isEnabled = enabled; }
    
    /**
     * Get the update rate in milliseconds.
     */
    uint16_t getUpdateRate() const { return updateRate; }
    /**
     * Set the update rate in milliseconds.
     */
    void setUpdateRate(uint16_t rate) { updateRate = rate; }
    
    /**
     * Check if the button is currently pressed.
     * @return true if the button is pressed, false otherwise.
     */
    bool isButtonPressed() const {
        return digitalRead(buttonPin) == LOW;
    }

private:
    MPU6886& imu;
    uint8_t buttonPin;
    float maxAccValue;
    float prevX = 0.0f;
    float prevY = 0.0f;
    bool isEnabled = false;
    uint16_t updateRate = 0;
    unsigned long lastUpdateTime = 0;
};