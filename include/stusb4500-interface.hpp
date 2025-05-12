#pragma once

#include <cstdint>

#include "esp_err.h"
#include "esp_log.h"

#include "I2CDevices.hpp"

namespace stusb4500 {

/**
 * @class INTERFACE
 * @brief Interface bas-niveau pour accéder aux registres du STUSB4500 via I2C.
 */
class INTERFACE {
public:
    explicit INTERFACE(I2CDevices& i2c_device) : i2c(i2c_device) {}

    esp_err_t read_register(uint8_t reg, uint8_t* data, size_t len) {
        esp_err_t err = ESP_FAIL;
        const int max_attempts = 3;
    
        for (int attempt = 0; attempt < max_attempts; ++attempt) {
            err = i2c.read(reg, data, len);
            if (err == ESP_OK) {
                return ESP_OK;
            }
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    
        ESP_LOGW("I2CWrapper", "Read failed at reg 0x%02X after %d attempts (err=0x%x)", reg, max_attempts, err);
        return err;
    }
    

    esp_err_t write_register(uint8_t reg, const uint8_t* data, size_t len) {
        //ESP_LOGI("STUSB4500-I2C", "I2C WRITE -> Reg: 0x%02X | Len: %d", reg, static_cast<int>(len));
        //ESP_LOG_BUFFER_HEX_LEVEL("STUSB4500-I2C", data, len, ESP_LOG_INFO);
    
        return i2c.write(reg, data, len);
    }

protected:
    I2CDevices& i2c;
};


} // namespace stusb4500
