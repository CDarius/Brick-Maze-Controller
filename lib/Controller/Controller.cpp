#include "Controller.hpp"

void Controller::begin() {
    pinMode(buttonPin, INPUT_PULLUP);

    maxAccValue = 0.8f;
    isEnabled = true;
    updateRate = 50;
}

bool Controller::getXY(float& x, float& y) {
    if (!isEnabled) {
        return false;
    }

    unsigned long currentTime = millis();
    if (currentTime - lastUpdateTime < updateRate) {
        return false;
    }

    lastUpdateTime = currentTime;

    float ax = 0.0f;
    float ay = 0.0f;
    float az = 0.0f;

    imu.getAccel(&ax, &ay, &az);

    ax = -ax;
    ay = -ay;
    az = -az;

    if (az < 0.0f) {
        x = prevX;
        y = prevY;
        return true;
    }

    x = constrain(ax, -maxAccValue, maxAccValue);
    y = constrain(ay, -maxAccValue, maxAccValue);

    prevX = x;
    prevY = y;

    return true;
}
