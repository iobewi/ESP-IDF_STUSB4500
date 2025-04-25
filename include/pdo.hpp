#pragma once

#include <cstdint>
#include "esp_log.h"
namespace stusb4500 {

    class PDO {
        public:
            explicit PDO(uint32_t raw = 0) : raw_(raw) {}
        
            uint16_t voltage_mv() const { return ((raw_ >> 10) & 0x3FF) * 50; }
            uint16_t current_ma() const { return (raw_ & 0x3FF) * 10; }
        
            bool dual_role_power() const     { return raw_ & (1 << 29); }
            bool higher_capability() const   { return raw_ & (1 << 28); }
            bool unconstrained_power() const { return raw_ & (1 << 27); }
            bool usb_comm_capable() const    { return raw_ & (1 << 26); }
        
            void log(size_t index = 0) const {
                ESP_LOGI("STUSB4500", "SRC_PDO[%zu]: %u mV @ %u mA (DRP=%s, CommCapable=%s)",
                    index,
                    voltage_mv(),
                    current_ma(),
                    dual_role_power() ? "Yes" : "No",
                    usb_comm_capable() ? "Yes" : "No");
            }
        
            uint32_t raw() const { return raw_; }
        
        private:
            uint32_t raw_;
    };
}