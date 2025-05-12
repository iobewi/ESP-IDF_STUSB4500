#include <cstdio>

#include "esp_log.h"

#include "nvm/stusb4500-nvm_data.hpp"

namespace stusb4500
{

    void NVMData::decode(const uint8_t *buffer)
    {
        bank1.decode(&buffer[8]);
        bank3.decode(&buffer[24]);
        bank4.decode(&buffer[32]);
    }

    void NVMData::encode(uint8_t *buffer) const
    {
        bank1.encode(&buffer[8]);
        bank3.encode(&buffer[24]);
        bank4.encode(&buffer[32]);
    }

    const std::array<uint8_t, 40> NVMData::default_nvm_map = {
        0x00, 0x00, 0xB0, 0xAA, 0x00, 0x45, 0x00, 0x00,
        0x10, 0x40, 0x9C, 0x1C, 0xFF, 0x01, 0x3C, 0xDF,
        0x02, 0x40, 0x0F, 0x00, 0x32, 0x00, 0xFC, 0xF1,
        0x00, 0x19, 0x56, 0xAF, 0xF5, 0x35, 0x5F, 0x00,
        0x00, 0x4B, 0x90, 0x21, 0x43, 0x00, 0x40, 0xFB};

    std::array<uint8_t, 40> NVMData::to_array() const
    {
        std::array<uint8_t, 40> result = default_nvm_map;
        encode(result.data());
        return result;
    }

    bool NVMData::equals(const std::array<uint8_t, 40> &other) const
    {
        return to_array() == other;
    }

    std::vector<std::tuple<size_t, uint8_t, uint8_t>> NVMData::diff(const std::array<uint8_t, 40> &other) const
    {
        std::vector<std::tuple<size_t, uint8_t, uint8_t>> differences;
        auto current = to_array();

        for (size_t i = 0; i < current.size(); ++i)
        {
            if (current[i] != other[i])
            {
                differences.emplace_back(i, other[i], current[i]);
            }
        }
        return differences;
    }

    void NVMData::print_diff(const std::array<uint8_t, 40> &other) const
    {
        auto diffs = diff(other);
        for (const auto &[i, before, after] : diffs)
        {
            ESP_LOGW("STUSB4500NVM", "Offset 0x%02X: 0x%02X -> 0x%02X",
                     static_cast<int>(i),
                     static_cast<int>(before),
                     static_cast<int>(after));
        }
        if (diffs.empty())
        {
            ESP_LOGW("STUSB4500NVM", "Aucune différence détectée dans la NVM.");
        }
    }

    void NVMData::log() const
    {
        auto buffer = to_array();

        ESP_LOGI("STUSB4500NVM", "Contenu brut NVM :");
        for (size_t i = 0; i < buffer.size(); i += 8)
        {
            char line[64];
            int len = snprintf(line, sizeof(line), "0x%02X: ", static_cast<int>(i));
            for (size_t j = 0; j < 8 && i + j < buffer.size(); ++j)
            {
                len += snprintf(line + len, sizeof(line) - len, "%02X ", buffer[i + j]);
            }
            ESP_LOGI("STUSB4500NVM", "%s", line);
        }
    }

} // namespace stusb4500
