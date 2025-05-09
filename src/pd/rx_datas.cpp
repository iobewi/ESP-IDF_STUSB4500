#include "pd/rx_datas.hpp"
#include "esp_log.h"

namespace stusb4500
{
    static const char *TAG = "STUSB450-RXDATAS";

    esp_err_t RXDatas::read()
    {
        uint8_t buffer[RXDatas::reg_len] = {0};

        esp_err_t err = read_register(RXDatas::reg_addr, buffer, sizeof(buffer));
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to read RXDatas at reg 0x%02X (err=0x%x)", RXDatas::reg_addr, err);
            return err;
        }

        decode(buffer, sizeof(buffer));
        return ESP_OK;
    }

    void RXDatas::decode(const uint8_t *buf, size_t len)
    {
        if (len >= 2)
        {
            header_ = buf[0] | (buf[1] << 8);
            size_t expected = num_objects() * 4;
            if (len >= 2 + expected)
            {
                for (size_t i = 0; i < num_objects() && i < 7; ++i)
                {
                    data_[i] = (buf[2 + i * 4]) |
                               (buf[3 + i * 4] << 8) |
                               (buf[4 + i * 4] << 16) |
                               (buf[5 + i * 4] << 24);
                }
            }
        }
    }

    uint16_t RXDatas::header() const
    {
        return header_;
    }

    uint8_t RXDatas::message_type() const
    {
        return header_ & 0x0F;
    }

    uint8_t RXDatas::num_objects() const
    {
        return (header_ >> 12) & 0x07;
    }

    bool RXDatas::is_data_message() const
    {
        return num_objects() > 0;
    }

    bool RXDatas::is_control_message() const
    {
        return num_objects() == 0;
    }

    PowerProfile RXDatas::get_pdo(size_t index) const
    {
        if (index < num_objects() && index < 7) {
            PowerProfile active_pdo = {};
            active_pdo.decode(data_[index], 0);
            return active_pdo;
        }
    
        return {};
    }

    void RXDatas::log() const
    {
        ESP_LOGI(TAG, "--- RXDatas ---");
        ESP_LOGI(TAG, "Header           : 0x%04X", header_);
        ESP_LOGI(TAG, "Message Type     : %u", message_type());
        ESP_LOGI(TAG, "Num Data Objects : %u", num_objects());
        for (size_t i = 0; i < num_objects() && i < 7; ++i)
        {
            ESP_LOGI(TAG, " ------------ PDO%zu ------------ ", (i)); 
            get_pdo(i).log();
        }
    }


} // namespace stusb4500