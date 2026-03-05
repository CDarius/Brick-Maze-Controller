#include "MPU6886.hpp"

// MPU6886 Register Addresses
#define MPU6886_WHO_AM_I        0x75
#define MPU6886_USER_CTRL       0x6A
#define MPU6886_FIFO_EN         0x23
#define MPU6886_PWR_MGMT_1      0x6B
#define MPU6886_PWR_MGMT_2      0x6C
#define MPU6886_CONFIG          0x1A
#define MPU6886_SMPLRT_DIV      0x19
#define MPU6886_GYRO_CONFIG     0x1B
#define MPU6886_ACCEL_CONFIG    0x1C
#define MPU6886_ACCEL_CONFIG2   0x1D
#define MPU6886_ACCEL_XOUT_H    0x3B
#define MPU6886_GYRO_XOUT_H     0x43
#define MPU6886_TEMP_OUT_H      0x41

// Default scale factors
#define GYRO_SCALE  (1.0f / 131.0f)    // ±250°/s range
#define TEMP_SCALE  (1.0f / 333.87f)   // Temperature scale
#define TEMP_OFFSET 21.0f              // Temperature at 0 register value

/**
 * Initialize the MPU6886 sensor with default configurations.
 * @return true if initialization was successful, false otherwise.
 */
bool MPU6886::begin(AccelScale accelScale) {
    delay(100);  // Give device time to stabilize
    
    // Reset the device first
    writeByte(MPU6886_PWR_MGMT_1, 0x80);
    delay(100);
    
    // Wake up the MPU6886 from sleep mode
    // Set clock source to AUTO, clear sleep bit
    writeByte(MPU6886_PWR_MGMT_1, 0x00);
    delay(50);
    
    // Now verify the device by reading WHO_AM_I
    uint8_t device_id = whoAmI();
    if (device_id != 0x19) {
        Serial.printf("WHO_AM_I returned 0x%02X, expected 0x19\n", device_id);
        return false;
    }
    
    // Configure power management 2 to enable gyro and accelerometer
    writeByte(MPU6886_PWR_MGMT_2, 0x00);
    delay(10);
    
    // Set DLPF to 41Hz bandwidth (recommended for gyro stability)
    writeByte(MPU6886_CONFIG, 0x03);

    if (!setSampleRateDivider(currentSampleRateDivider)) {
        return false;
    }

    if (!setAccelFilter(currentAccelFilter)) {
        return false;
    }
    
    // Configure gyroscope range to ±250°/s (0x00)
    writeByte(MPU6886_GYRO_CONFIG, 0x00);
    
    if (!setAccelScale(accelScale)) {
        return false;
    }
    delay(10);
    
    // Disable FIFO
    writeByte(MPU6886_USER_CTRL, 0x00);
    writeByte(MPU6886_FIFO_EN, 0x00);
    
    return true;
}

bool MPU6886::setAccelScale(AccelScale accelScale) {
    float scaleFactor;

    switch (accelScale) {
        case AccelScale::RANGE_2G:
            scaleFactor = 1.0f / 16384.0f;
            break;
        case AccelScale::RANGE_4G:
            scaleFactor = 1.0f / 8192.0f;
            break;
        case AccelScale::RANGE_8G:
            scaleFactor = 1.0f / 4096.0f;
            break;
        case AccelScale::RANGE_16G:
            scaleFactor = 1.0f / 2048.0f;
            break;
        default:
            return false;
    }

    if (!writeByte(MPU6886_ACCEL_CONFIG, static_cast<uint8_t>(accelScale))) {
        return false;
    }

    accelScaleFactor = scaleFactor;
    currentAccelScale = accelScale;

    return true;
}

float MPU6886::getAccelGRange() const {
    switch (currentAccelScale) {
        case AccelScale::RANGE_2G:
            return 2.0f;
        case AccelScale::RANGE_4G:
            return 4.0f;
        case AccelScale::RANGE_8G:
            return 8.0f;
        case AccelScale::RANGE_16G:
            return 16.0f;
        default:
            return 2.0f;
    }
}

bool MPU6886::setAccelFilter(AccelFilter filter) {
    if (!writeByte(MPU6886_ACCEL_CONFIG2, static_cast<uint8_t>(filter))) {
        return false;
    }

    currentAccelFilter = filter;
    return true;
}

bool MPU6886::setSampleRateDivider(uint8_t divider) {
    if (!writeByte(MPU6886_SMPLRT_DIV, divider)) {
        return false;
    }

    currentSampleRateDivider = divider;
    return true;
}

MPU6886::AccelScale MPU6886::getAccelScale() const {
    return currentAccelScale;
}

MPU6886::AccelFilter MPU6886::getAccelFilter() const {
    return currentAccelFilter;
}

uint8_t MPU6886::getSampleRateDivider() const {
    return currentSampleRateDivider;
}

/**
 * Read WHO_AM_I register to verify device identity.
 * @return Device ID (should be 0x19 for MPU6886)
 */
uint8_t MPU6886::whoAmI(void) {
    uint8_t data;
    readByte(MPU6886_WHO_AM_I, &data);
    return data;
}

/**
 * Read accelerometer values and convert to g (gravitational acceleration).
 * @param ax Pointer to store X-axis acceleration in g
 * @param ay Pointer to store Y-axis acceleration in g
 * @param az Pointer to store Z-axis acceleration in g
 */
void MPU6886::getAccel(float* ax, float* ay, float* az) {
    uint8_t buf[6];
    readBytes(MPU6886_ACCEL_XOUT_H, buf, 6);
    
    int16_t ax_raw = (int16_t)((buf[0] << 8) | buf[1]);
    int16_t ay_raw = (int16_t)((buf[2] << 8) | buf[3]);
    int16_t az_raw = (int16_t)((buf[4] << 8) | buf[5]);
    
    *ax = (float)ax_raw * accelScaleFactor;
    *ay = (float)ay_raw * accelScaleFactor;
    *az = (float)az_raw * accelScaleFactor;
}

/**
 * Read gyroscope values and convert to degrees per second.
 * @param gx Pointer to store X-axis rotation in °/s
 * @param gy Pointer to store Y-axis rotation in °/s
 * @param gz Pointer to store Z-axis rotation in °/s
 */
void MPU6886::getGyro(float* gx, float* gy, float* gz) {
    uint8_t buf[6];
    readBytes(MPU6886_GYRO_XOUT_H, buf, 6);
    
    int16_t gx_raw = (int16_t)((buf[0] << 8) | buf[1]);
    int16_t gy_raw = (int16_t)((buf[2] << 8) | buf[3]);
    int16_t gz_raw = (int16_t)((buf[4] << 8) | buf[5]);
    
    *gx = (float)gx_raw * GYRO_SCALE;
    *gy = (float)gy_raw * GYRO_SCALE;
    *gz = (float)gz_raw * GYRO_SCALE;
}

/**
 * Read temperature value and convert to Celsius.
 * @param t Pointer to store temperature in °C
 */
void MPU6886::getTemp(float *t) {
    uint8_t buf[2];
    readBytes(MPU6886_TEMP_OUT_H, buf, 2);
    
    int16_t temp_raw = (int16_t)((buf[0] << 8) | buf[1]);
    
    *t = TEMP_OFFSET + ((float)temp_raw / TEMP_SCALE);
}
