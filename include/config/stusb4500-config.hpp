#pragma once

#include <cstdint>
#include "esp_log.h"

#include "stusb4500-interface.hpp"
#include "config/stusb4500-config_types.hpp"

namespace stusb4500
{
    class Config : public INTERFACE
    {
    public:
        Config(I2CDevices &dev, 
            const ConfigParams& params = {});
        
           
        esp_err_t get_alert_status_mask() ;
        esp_err_t set_alert_status_mask();

        ConfigParams& datas() { return params_; };
        const ConfigParams& datas() const { return params_; };

    private:
        ConfigParams params_;
        inline static const char *TAG = "STUSB4500-CONFIG";
    };

} // namespace stusb4500
