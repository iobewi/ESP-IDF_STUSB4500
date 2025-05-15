#include "pd/stusb4500-rdo.hpp"

#include "esp_log.h"

namespace stusb4500
{
    static const char *TAG = "STUSB4500-RDO";

    esp_err_t RDO::read()
    {
        uint8_t buffer[RDO::reg_len] = {0};

        esp_err_t err = read_register(RDO::reg_addr, buffer, sizeof(buffer));
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to read RDO at reg 0x%02X (err=0x%x)", RDO::reg_addr, err);
            return err;
        }

        decode(buffer, sizeof(buffer));
        return ESP_OK;
    }

    void RDO::decode(const uint8_t *buf, size_t len)
    {
        if (len < 4)
            return;
        raw_ = buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);
    }
    
    uint32_t RDO::encode() const { return raw_; }

    uint8_t RDO::obj_position() const { return (raw_ >> 28) & 0x07; }
    bool RDO::giveback() const { return raw_ & (1 << 27); }
    bool RDO::capability_mismatch() const { return raw_ & (1 << 26); }
    bool RDO::usb_comm_capable() const { return raw_ & (1 << 25); }
    bool RDO::no_usb_suspend() const { return raw_ & (1 << 24); }
    bool RDO::unchunked_ext() const { return raw_ & (1 << 23); }
    uint16_t RDO::operating_ma() const { return ((raw_ >> 10) & 0x3FF) * 10; }
    uint16_t RDO::max_operating_ma() const { return (raw_ & 0x3FF) * 10; }

    void RDO::log() const
    {
        if (obj_position() == 0)
        {
            ESP_LOGI(TAG, "No active RDO.");
            return;
        }
        ESP_LOGI(TAG, "--- Requested RDO ---");
        ESP_LOGI(TAG, "  Pos               : %u", obj_position());
        ESP_LOGI(TAG, "  GiveBack          : %s", giveback() ? "Yes" : "No");
        ESP_LOGI(TAG, "  CapMismatch       : %s", capability_mismatch() ? "Yes" : "No");
        ESP_LOGI(TAG, "  USB Comm Capable  : %s", usb_comm_capable() ? "Yes" : "No");
        ESP_LOGI(TAG, "  No Suspend        : %s", no_usb_suspend() ? "Yes" : "No");
        ESP_LOGI(TAG, "  Unchunked Ext Msg : %s", unchunked_ext() ? "Yes" : "No");
        ESP_LOGI(TAG, "  Current           : %u mA", operating_ma());
        ESP_LOGI(TAG, "  Max Current       : %u mA", max_operating_ma());
    }

} // namespace stusb4500