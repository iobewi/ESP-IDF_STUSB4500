#include <cstdint>
#include "common_types.hpp"

#include "esp_log.h"

namespace stusb4500
{
    static const char *TAG = "STUSB450-PDO";
        void PowerProfile::decode(uint32_t raw, size_t index)
    {

        // --- Extraction des champs physiques ---
        uint16_t voltage_step = (raw >> 10) & 0x3FF;
        uint16_t current_step = raw & 0x3FF;

        pdos[index].voltage_mv = voltage_step * 50;
        pdos[index].current_ma = current_step * 10;

        // --- Extraction des bits de configuration partagée ---
        dual_role_power = raw & (1 << 29);
        higher_capability = raw & (1 << 28);
        unconstrained_power = raw & (1 << 27);
        usb_comm_capable = raw & (1 << 26);

        frs = static_cast<FastRoleSwap>((raw >> 23) & 0x03);
    }

    uint32_t PowerProfile::encode(size_t index) const
    {
        uint32_t raw = 0;

        const uint16_t voltage_step = pdos[index].voltage_mv / 50;
        const uint16_t current_step = pdos[index].current_ma / 10;

        if (dual_role_power)
            raw |= (1 << 29);
        if (higher_capability)
            raw |= (1 << 28);
        if (unconstrained_power)
            raw |= (1 << 27);
        if (usb_comm_capable)
            raw |= (1 << 26);

        raw |= (static_cast<uint32_t>(frs) & 0x03) << 23;
        raw |= (static_cast<uint32_t>(voltage_step) & 0x3FF) << 10;
        raw |= (static_cast<uint32_t>(current_step) & 0x3FF);

        return raw;
    }

    void PowerProfile::log() const
    {
        ESP_LOGI(TAG, "====== Power Profile ======");
        ESP_LOGI(TAG, "Common Flags:");
        ESP_LOGI(TAG, "  CommCapable     : %s", usb_comm_capable ? "Yes" : "No");
        ESP_LOGI(TAG, "  DualRole        : %s", dual_role_power ? "Yes" : "No");
        ESP_LOGI(TAG, "  Higher Capability: %s", higher_capability ? "Yes" : "No");
        ESP_LOGI(TAG, "  UnconsPower     : %s", unconstrained_power ? "Yes" : "No");
        ESP_LOGI(TAG, "  FRS             : 0x%02X", static_cast<uint8_t>(frs));
        ESP_LOGI(TAG, "PDO Count         : %u", pdo_number);
        
        for (size_t i = 0; i < pdos.size(); ++i)
        {
            ESP_LOGI(TAG, "--- PDO #%zu ---", i);
            pdos[i].log();
        }
    
        ESP_LOGI(TAG, "===========================");
    }

    std::string PowerProfile::to_json() const
    {
        std::string json = "{";

        json += "\"usb_comm_capable\": " + std::string(usb_comm_capable ? "true" : "false") + ",";
        json += "\"dual_role_power\": " + std::string(dual_role_power ? "true" : "false") + ",";
        json += "\"higher_capability\": " + std::string(higher_capability ? "true" : "false") + ",";
        json += "\"unconstrained_power\": " + std::string(unconstrained_power ? "true" : "false") + ",";
        json += "\"frs\": " + std::to_string(static_cast<uint8_t>(frs)) + ",";
        json += "\"pdo_number\": " + std::to_string(pdo_number) + ",";

        json += "\"pdos\": [";
        for (size_t i = 0; i < pdos.size(); ++i)
        {
            json += pdos[i].to_json();
            if (i + 1 < pdos.size())
                json += ",";
        }
        json += "]";

        json += "}";

        return json;
    }


    void PDObjectProfile::log() const{      
        ESP_LOGI(TAG, "  Voltage : %u mV, Current : %u mA", voltage_mv, current_ma);
    }

    std::string PDObjectProfile::to_json() const
    {
        return std::string("{") +
            "\"voltage_mv\": " + std::to_string(voltage_mv) + "," +
            "\"current_ma\": " + std::to_string(current_ma) +
            "}";
    }

}