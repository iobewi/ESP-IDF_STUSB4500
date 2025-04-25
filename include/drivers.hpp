#pragma once

#include "I2CDevices.hpp"
#include "esp_err.h"
#include <cstdint>

namespace stusb4500 {

/**
 * @class INTERFACE
 * @brief Interface bas-niveau pour accéder aux registres du STUSB4500 via I2C.
 */
class INTERFACE {
public:
    explicit INTERFACE(I2CDevices& i2c_device) : i2c(i2c_device) {}

    esp_err_t read_register(uint8_t reg, uint8_t* data, size_t len) {
        return i2c.read(reg, data, len);
    }

    esp_err_t write_register(uint8_t reg, const uint8_t* data, size_t len) {
        return i2c.write(reg, data, len);
    }

private:
    I2CDevices& i2c;
};

} // namespace stusb4500
