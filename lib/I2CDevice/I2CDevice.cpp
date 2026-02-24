#include "I2CDevice.hpp"

bool I2CDevice::begin() {
    wire.beginTransmission(addr);
    uint8_t error = wire.endTransmission();
    if (error == 0) {
        return true;
    } else {
        return false;
    }
}
