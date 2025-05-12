#include "pd/stusb4500-pdo.hpp"

#include "esp_log.h"

namespace stusb4500
{
    static const char *TAG = "STUSB4500-PDO";

    PDO::PDO(I2CDevices &dev,
            const size_t index,
            const PDObjectProfile& pdo_profile)
        : INTERFACE(dev),
        index_(index)
        {
        power_.pdos.clear();
        power_.pdos.push_back(pdo_profile);
        power_.pdo_number = 1;
        };

    esp_err_t PDO::read()
    {
        if (index_ < 1 || index_ > 3)
        {
            ESP_LOGE(TAG, "Invalid PDO index %u (must be 1, 2 or 3)", index_);
            return ESP_ERR_INVALID_ARG;
        }

        uint8_t buffer[PDO::reg_len] = {0};
        uint8_t reg = PDO::base_reg_addr + (index_ - 1) * PDO::reg_len;

        esp_err_t err = read_register(reg, buffer, sizeof(buffer));
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to read PDO[%u] at reg 0x%02X (err=0x%x)", index_, reg, err);
            return err;
        }

        uint32_t raw = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);

        power_.decode(raw,0);

        return ESP_OK;
    }

    esp_err_t PDO::write()
    {
        if (index_ < 1 || index_ > 3)
        {
            ESP_LOGE(TAG, "Invalid PDO index %u (must be 1, 2 or 3)", index_);
            return ESP_ERR_INVALID_ARG;
        }

        uint32_t raw = power_.encode(0);

        uint8_t buffer[4];
        buffer[0] = raw & 0xFF;
        buffer[1] = (raw >> 8) & 0xFF;
        buffer[2] = (raw >> 16) & 0xFF;
        buffer[3] = (raw >> 24) & 0xFF;

        uint8_t reg = PDO::base_reg_addr + (index_ - 1) * 4;

        esp_err_t err = write_register(reg, buffer, sizeof(buffer));
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to write PDO[%u] at reg 0x%02X (err=0x%x)", index_, reg, err);
            return err;
        }
        return ESP_OK;
    }
} // namespace stusb4500