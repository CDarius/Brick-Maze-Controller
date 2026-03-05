#pragma once

#include <I2CDevice.hpp>

class MPU6886 : public I2CDevice {
public:
    enum class AccelScale : uint8_t {
        RANGE_2G = 0x00,
        RANGE_4G = 0x08,
        RANGE_8G = 0x10,
        RANGE_16G = 0x18
    };

    enum class AccelFilter : uint8_t {
        BW_460HZ = 0x00,
        BW_184HZ = 0x01,
        BW_92HZ = 0x02,
        BW_41HZ = 0x03,
        BW_20HZ = 0x04,
        BW_10HZ = 0x05,
        BW_5HZ = 0x06
    };

    MPU6886(TwoWire &wire, uint8_t address = 0x68) : I2CDevice(wire, address) 
    {        
    };
    
    bool begin(AccelScale accelScale = AccelScale::RANGE_2G);
    uint8_t whoAmI(void);
    bool setAccelScale(AccelScale accelScale);
    float getAccelGRange() const;
    bool setAccelFilter(AccelFilter filter);
    bool setSampleRateDivider(uint8_t divider);
    AccelScale getAccelScale() const;
    AccelFilter getAccelFilter() const;
    uint8_t getSampleRateDivider() const;
    void getAccel(float* ax, float* ay, float* az);
    void getGyro(float* gx, float* gy, float* gz);
    void getTemp(float *t);

private:
    float accelScaleFactor = 1.0f / 16384.0f;
    AccelScale currentAccelScale = AccelScale::RANGE_2G;
    AccelFilter currentAccelFilter = AccelFilter::BW_10HZ;
    uint8_t currentSampleRateDivider = 19;

};
