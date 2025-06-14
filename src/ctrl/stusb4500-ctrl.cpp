#include "ctrl/stusb4500-ctrl.hpp"
#include "esp_log.h"
#define RETURN_IF_ERROR(x)                          \
    do {                                             \
        esp_err_t __err_rc = (x);                   \
        if (__err_rc != ESP_OK) {                   \
            ESP_LOGE("RETURN_IF_ERROR",             \
                     "%s failed at %s:%d → %s",     \
                     #x, __FILE__, __LINE__,        \
                     esp_err_to_name(__err_rc));    \
            return __err_rc;                        \
        }                                            \
    } while (0)

namespace stusb4500
{
    static const char *TAG = "STUSB4500-CTRL";

    esp_err_t CTRL::ready()
    {
        uint8_t value;
        RETURN_IF_ERROR(read_u8(DEVICE_ID_REG, value));
        if (value != DEVICE_ID_VALUE) return ESP_ERR_INVALID_RESPONSE;
        return ESP_OK;
    }

    esp_err_t CTRL::send_soft_reset()
    {
        esp_err_t err = write_register(TX_HEADER_LOW, &SOFT_RESET_HEADER, 1);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to write TX_HEADER_LOW (err=0x%x)", err);
            return err;
        }

        err = write_register(PD_COMMAND_CTRL, &SOFT_RESET_COMMAND, 1);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to write PD_COMMAND_CTRL (err=0x%x)", err);
            return err;
        }

        ESP_LOGI(TAG, "USB PD Soft Reset command sent.");
        return ESP_OK;
    }

    esp_err_t CTRL::send_hard_reset()
    {
        esp_err_t err = write_register(PD_COMMAND_CTRL, &HARD_RESET_COMMAND, 1);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to write HARD_RESET to PD_COMMAND_CTRL (err=0x%x)", err);
            return err;
        }

        ESP_LOGI(TAG, "USB PD Hard Reset command sent.");
        return ESP_OK;
    }

    esp_err_t CTRL::update_pdo_number(uint8_t count)
    {
        if (count < 1 || count > 3)
        {
            ESP_LOGE(TAG, "Invalid PDO count: %u", count);
            return ESP_ERR_INVALID_ARG;
        }

        esp_err_t err = write_register(PDO_NUM_REG, &count, 1);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to send UPDATE_VALID_PDO_NUMBER command (err=0x%x)", err);
            return err;
        }

        ESP_LOGI(TAG, "Updated VALID_PDO_NUMBER to %u", count);
        return ESP_OK;
    }

} // namespace stusb4500
