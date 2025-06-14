#pragma once

#include "esp_err.h"
#include <cstdint>

#include "stusb4500-interface.hpp"

namespace stusb4500
{
    class CTRL : public INTERFACE {
        public:
            explicit CTRL(I2CDevices& dev) : INTERFACE(dev) {}

        esp_err_t ready();
        esp_err_t send_soft_reset();
        esp_err_t send_hard_reset();
        esp_err_t update_pdo_number(uint8_t count);
    
        private:
        inline static const char *TAG = "STUSB4500-CTRL";
        static constexpr uint8_t TX_HEADER_LOW      = 0x51;
        static constexpr uint8_t PD_COMMAND_CTRL    = 0x1A;
        static constexpr uint8_t SOFT_RESET_HEADER  = 0x0D;
        static constexpr uint8_t SOFT_RESET_COMMAND = 0x26;
        static constexpr uint8_t HARD_RESET_COMMAND = 0x05;
        static constexpr uint8_t PDO_NUM_REG        = 0x70;
        static constexpr uint8_t DEVICE_ID_REG      = 0x2F;
        static constexpr uint8_t DEVICE_ID_VALUE    = 0x25;
    };

} // namespace stusb4500
