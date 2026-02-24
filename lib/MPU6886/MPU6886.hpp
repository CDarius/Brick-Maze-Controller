#pragma once

#include <I2CDevice.hpp>

class MPU6886 : public I2CDevice {
public:
    MPU6886(TwoWire &wire, uint8_t address = 0x68) : I2CDevice(wire, address) 
    {        
    };

    bool begin(void);
    uint8_t whoAmI(void);
    void getAccel(float* ax, float* ay, float* az);
    void getGyro(float* gx, float* gy, float* gz);
    void getTemp(float *t);

};
