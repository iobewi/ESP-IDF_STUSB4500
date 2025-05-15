#include "config/stusb4500-config_types.hpp"
#include "cJSON.h"
#include "esp_log.h"

namespace stusb4500
{
    static const char *TAG = "STUSB4500-CONFIG";
    inline std::string json_bool(const char *key, bool value)
    {
        return "\"" + std::string(key) + "\": " + (value ? "true" : "false");
    }
    
    AlertStatus1MaskRegister::AlertStatus1MaskReg AlertStatus1MaskRegister::get_values() const
    {
        AlertStatus1MaskReg values = {};
        values.port_status_al_mask = raw_ & (1 << 6);
        values.typec_monitoring_status_al_mask = raw_ & (1 << 5);
        values.cc_hw_fault_status_al_mask = raw_ & (1 << 4);
        values.prt_status_al_mask = raw_ & (1 << 1);
        return values;
    }

    void AlertStatus1MaskRegister::log() const
    {
        AlertStatus1MaskReg values = get_values();
        ESP_LOGI(TAG, "ALERT_STATUS_1_MASK: PORT_STATUS_AL_MASK=%s, TYPEC_MONITORING_STATUS_AL_MASK=%s, CC_HW_FAULT_STATUS_AL_MASK=%s, PRT_STATUS_AL_MASK=%s",
                 values.port_status_al_mask ? "MASKED" : "UNMASKED",
                 values.typec_monitoring_status_al_mask ? "MASKED" : "UNMASKED",
                 values.cc_hw_fault_status_al_mask ? "MASKED" : "UNMASKED",
                 values.prt_status_al_mask ? "MASKED" : "UNMASKED");
    }

    std::string AlertStatus1MaskRegister::to_json() const
    {
        AlertStatus1MaskReg values = get_values();
        return std::string("{") +
               json_bool("port_status_al_mask", values.port_status_al_mask) + "," +
               json_bool("typec_monitoring_status_al_mask", values.typec_monitoring_status_al_mask) + "," +
               json_bool("cc_hw_fault_status_al_mask", values.cc_hw_fault_status_al_mask) + "," +
               json_bool("prt_status_al_mask", values.prt_status_al_mask) +
               "}";
    }

    ConfigParams::ConfigParams()
    {
        power_.pdos.resize(3);
    };

    std::string  ConfigParams::to_string(GPIOFunction func)
    {
        switch (func)
        {
            case GPIOFunction::SWCtrl:         return "SWCtrl";
            case GPIOFunction::ErrorRecovery:  return "ErrorRecovery";
            case GPIOFunction::Debug:          return "Debug";
            case GPIOFunction::SinkPower:      return "SinkPower";
            default:                           return "UNKNOWN";
        }
    }

    std::string  ConfigParams::to_string(PowerOkConfig config)
    {
        switch (config)
        {
            case PowerOkConfig::CONFIG_1:        return "CONFIG_1";
            case PowerOkConfig::NOT_APPLICABLE:  return "NOT_APPLICABLE";
            case PowerOkConfig::CONFIG_2:        return "CONFIG_2";
            case PowerOkConfig::CONFIG_3:        return "CONFIG_3";
            default:                             return "UNKNOWN";
        }
    }


    void ConfigParams::log() const
    {
        ESP_LOGI(TAG, "========== Configuration STUSB4500 ==========");

        // GPIO Function
        ESP_LOGI(TAG, "GPIO Function       : %s", to_string(gpio_function).c_str());

         // Power OK Config
         ESP_LOGI(TAG, "Power OK Config     : %s", to_string(power_ok).c_str());

        // Discharge settings
        ESP_LOGI(TAG, "Discharge to 0V     : %u", discharge_.time_to_0v);
        ESP_LOGI(TAG, "Discharge to PDO    : %u", discharge_.time_to_pdo);
        ESP_LOGI(TAG, "Discharge disable   : %s", discharge_.disable ? "true" : "false");

        // Common flags

        ESP_LOGI(TAG, "Common Flags        : usb_comm=%d, dual_role=%d, higher_cap=%d, unconstrained=%d, FRS=%d",
                 power_.usb_comm_capable,
                 power_.dual_role_power,
                 power_.higher_capability,
                 power_.unconstrained_power,
                 static_cast<int>(power_.frs));

        // Nombre de PDO
        ESP_LOGI(TAG, "Nombre de PDO       : %u", power_.pdo_number);

        // Flex current
        ESP_LOGI(TAG, "Flex current target : %u mA",
                 power_.flex_current_ma);

        // PDOs détaillés
        for (size_t i = 0; i < power_.pdo_number; ++i)
        {
            const auto &pdo = power_.pdos[i];
            ESP_LOGI(TAG, "--- PDO #%d ---", static_cast<int>(i + 1));
            ESP_LOGI(TAG, "  Voltage          : %u mV", pdo.voltage_mv);
            ESP_LOGI(TAG, "  Current          : %u mA", pdo.current_ma);
            ESP_LOGI(TAG, "  VBUS thresholds  : [%u%% - %u%%]",
                     pdo.vbus_monitor.lower_percent,
                     pdo.vbus_monitor.upper_percent);
        }

        // Autres
        ESP_LOGI(TAG, "Power only 5V       : %s", power_only_5v ? "true" : "false");
        ESP_LOGI(TAG, "Request Src Current : %s", req_src_current ? "true" : "false");
        ESP_LOGI(TAG, "Alert mask          : 0x%02X", alert_mask);
        ESP_LOGI(TAG, "=============================================");
    }

    std::string ConfigParams::to_json() const
    {
        cJSON *root = cJSON_CreateObject();
    
        // GPIO function
        cJSON_AddStringToObject(root, "gpio_function", to_string(gpio_function).c_str());

        // Power OK config
        cJSON_AddStringToObject(root, "power_ok", to_string(power_ok).c_str());
            
        // Discharge
        cJSON *discharge = cJSON_CreateObject();
        cJSON_AddNumberToObject(discharge, "time_to_0v", discharge_.time_to_0v);
        cJSON_AddNumberToObject(discharge, "time_to_pdo", discharge_.time_to_pdo);
        cJSON_AddBoolToObject(discharge, "disable", discharge_.disable);
        cJSON_AddItemToObject(root, "discharge", discharge);
    
        // Power profile
        cJSON *profile = cJSON_CreateObject();
        cJSON_AddBoolToObject(profile, "usb_comm_capable", power_.usb_comm_capable);
        cJSON_AddBoolToObject(profile, "dual_role_power", power_.dual_role_power);
        cJSON_AddBoolToObject(profile, "higher_capability", power_.higher_capability);
        cJSON_AddBoolToObject(profile, "unconstrained_power", power_.unconstrained_power);
        cJSON_AddNumberToObject(profile, "frs", static_cast<int>(power_.frs));
        cJSON_AddNumberToObject(profile, "pdo_number", power_.pdo_number);
        cJSON_AddNumberToObject(profile, "flex_current_ma", power_.flex_current_ma);
    
        cJSON *pdos_array = cJSON_CreateArray();
        for (size_t i = 0; i < power_.pdo_number; ++i)
        {
            const auto& pdo = power_.pdos[i];
            cJSON *pdo_obj = cJSON_CreateObject();
            cJSON_AddNumberToObject(pdo_obj, "voltage_mv", pdo.voltage_mv);
            cJSON_AddNumberToObject(pdo_obj, "current_ma", pdo.current_ma);
    
            cJSON *vbus = cJSON_CreateObject();
            cJSON_AddNumberToObject(vbus, "lower_percent", pdo.vbus_monitor.lower_percent);
            cJSON_AddNumberToObject(vbus, "upper_percent", pdo.vbus_monitor.upper_percent);
            cJSON_AddItemToObject(pdo_obj, "vbus_monitor", vbus);
    
            cJSON_AddItemToArray(pdos_array, pdo_obj);
        }
        cJSON_AddItemToObject(profile, "pdos", pdos_array);
        cJSON_AddItemToObject(root, "power_profile", profile);
    
        // Flags & autres
        cJSON_AddBoolToObject(root, "power_only_5v", power_only_5v);
        cJSON_AddBoolToObject(root, "req_src_current", req_src_current);
    
        // Conversion en string
        char* json_str = cJSON_PrintUnformatted(root);
        std::string result(json_str);
    
        // Nettoyage
        cJSON_Delete(root);
        free(json_str);
    
        return result;
    }
    
};