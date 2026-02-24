#pragma once

#include <Arduino.h>
#include <Wire.h>

/**
 * I2CDevice is a base class for communicating with I2C devices. It provides methods to initialize communication,
 * write bytes to a device register, and read bytes from a device register. Derived classes can implement specific
 * functionality for different I2C devices by utilizing these basic read/write operations.
 */
class I2CDevice {
    protected:
        uint8_t addr;
        TwoWire& wire;

        /**
         * Write multiple bytes to the device starting from a specific register.
         * @param reg The starting register address to write to.
         * @param buffer Pointer to the data buffer to be written.
         * @param length The number of bytes to write from the buffer.
         * @return true if the write operation was successful, false otherwise.
         */
        inline bool writeBytes(uint8_t reg, uint8_t *buffer, uint8_t length) {
            wire.beginTransmission(addr);
            wire.write(reg);
            for (uint8_t i = 0; i < length; i++) {
                wire.write(*(buffer + i));
            }
            if (wire.endTransmission() == 0) return true;
            return false;
        }

        /**
         * Read multiple bytes from the device starting from a specific register.
         * @param reg The starting register address to read from.
         * @param buffer Pointer to the data buffer where the read bytes will be stored.
         * @param length The number of bytes to read into the buffer.
         * @return true if the read operation was successful, false otherwise.
         */
        inline bool readBytes(uint8_t reg, uint8_t *buffer, uint8_t length) {
            uint8_t index = 0;
            wire.beginTransmission(addr);
            wire.write(reg);
            wire.endTransmission(false);
            if (wire.requestFrom(addr, length)) {
                for (uint8_t i = 0; i < length; i++) {
                    buffer[index++] = wire.read();
                }
                return true;
            }
            return false;
        }

        /**
         * Read a 16-bit unsigned integer from the device starting from a specific register.
         * @param reg The starting register address to read from.
         * @param value Pointer to the variable where the read value will be stored.
         * @return true if the read operation was successful, false otherwise.
         */
        inline bool readUint16(uint8_t reg, uint16_t *value) {
            uint8_t read_buf[2];
            bool result         = readBytes(reg, read_buf, 2);
            *value              = (read_buf[1] << 8) | read_buf[0];
            return result;
        }

        /**
         * Write a 16-bit unsigned integer to the device starting from a specific register.
         * @param reg The starting register address to write to.
         * @param value The 16-bit value to write.
         * @return true if the write operation was successful, false otherwise.
         */
        inline bool writeUint16(uint8_t reg, uint16_t value) {
            uint8_t write_buf[2];
            write_buf[0] = value & 0xff;
            write_buf[1] = value >> 8;
            return writeBytes(reg, write_buf, 2);
        }

        /**
         * Read a single byte from the device starting from a specific register.
         * @param reg The register address to read from.
         * @param data Pointer to the variable where the read byte will be stored.
         * @return true if the read operation was successful, false otherwise.
         */
        inline bool readByte(uint8_t reg, uint8_t *data) {
            wire.beginTransmission(addr);
            wire.write(reg);
            wire.endTransmission(false);  // Keep bus active for repeated start

            if (wire.requestFrom(addr, (uint8_t)1)) {
                *data = wire.read();
                return true;
            }

            return false;
        }

        /**
         * Write a single byte to the device starting from a specific register.
         * @param reg The register address to write to.
         * @param data The byte to write.
         * @return true if the write operation was successful, false otherwise.
         */
        inline bool writeByte(uint8_t reg, uint8_t data) {
            wire.beginTransmission(addr);
            wire.write(reg);
            wire.write(data);
            return (wire.endTransmission() == 0);
        }

   public:
        /** 
         * Constructor for I2CDevice.
         * @param wire Reference to the TwoWire instance (I2C bus) to use for communication.
         * @param addr The I2C address of the device.
        */
        I2CDevice(TwoWire &wire, uint8_t addr) : wire(wire), addr(addr) 
        {
        }

        /**
         * Initialize communication with the I2C device by checking if it acknowledges its address.
         * @return true if the device acknowledges its address, false otherwise.
         */
        bool begin();
};