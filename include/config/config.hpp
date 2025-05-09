#pragma once

#include <cstdint>
#include "esp_log.h"

#include "common_types.hpp"

namespace stusb4500
{
    class Config
    {
    public:
        Config(const PDObjectProfile& p1 = {},
            const PDObjectProfile& p2 = {},
            const PDObjectProfile& p3 = {});
        
        // Accès aux blocs
        PowerProfile &power() { return power_; }
        const PowerProfile &power() const { return power_; }

        DischargeSettings &discharge() { return discharge_; }
        const DischargeSettings &discharge() const { return discharge_; }

        // Autres paramètres
        GPIOFunction gpio_function = GPIOFunction::ErrorRecovery;
        PowerOkConfig power_ok = PowerOkConfig::CONFIG_2;
        bool req_src_current = false;
        bool power_only_5v = false;
        uint8_t alert_mask = 0;

        void log() const;
        std::string to_json() const;

    private:
        PowerProfile power_;
        DischargeSettings discharge_ = {};
    };

} // namespace stusb4500
