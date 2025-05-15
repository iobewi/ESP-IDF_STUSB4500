#include "config/stusb4500-config.hpp"
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
    
    Config::Config(I2CDevices &dev, const ConfigParams &params)
        : INTERFACE(dev),
          params_(params) {}

    esp_err_t Config::get_alert_status_mask(){
            uint8_t val = 0;
            RETURN_IF_ERROR(read_register(params_.alert_mask.reg_addr, &val, 1));
            params_.alert_mask.set_raw(val);
            return ESP_OK;
    }

    esp_err_t Config::set_alert_status_mask(){
            uint8_t val = params_.alert_mask.get_raw();
            RETURN_IF_ERROR(write_register(params_.alert_mask.reg_addr, &val, 1));
            return ESP_OK;
    }

};