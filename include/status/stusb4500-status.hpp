#pragma once

#include "esp_err.h"
#include <cstdint>

#include "stusb4500-interface.hpp"
#include "status/stusb4500-status_types.hpp"

namespace stusb4500
{
    class STATUS : public INTERFACE
    {
    public:
        explicit STATUS(I2CDevices &dev) : INTERFACE(dev) {}

        StateStatusRegister policy_engine_state;                  // 0x29
        AlertStatus1Register alert_status_1;                      // 0x0B
        PortStatus0Register port_status_0;                        // 0x0D
        PortStatus1Register port_status_1;                        // 0x0E
        TypeCMonitoringStatus0Register typec_monitoring_status_0; // 0x0F
        TypeCMonitoringStatus1Register typec_monitoring_status_1; // 0x10
        CCStatusRegister cc_status;                               // 0x11
        CCHwFaultStatus0Register cc_hw_fault_0;                   // 0x12
        CCHwFaultStatus1Register cc_hw_fault_1;                   // 0x13
        PDTypeCStatusRegister pd_typec_status;                    // 0x14
        TypeCStatusRegister typec_status;                         // 0x15
        PRTStatusRegister prt_status;                             // 0x16

        template <typename T>
        esp_err_t read_register_and_decode(const char *name, T &reg)
        {
            uint8_t val = 0;
            esp_err_t err = read_register(reg.reg_addr, &val, 1);
            if (err != ESP_OK)
            {
                ESP_LOGE(TAG, "Failed to read %s (err=0x%x)", name, err);
                return err;
            }
            reg.set_raw(val);
            return ESP_OK;
        }

        esp_err_t read_policy_engine_state() { return read_register_and_decode("Policy Engine State", policy_engine_state); }
        esp_err_t read_alert_status() { return read_register_and_decode("ALERT_STATUS_1", alert_status_1); }
        esp_err_t read_port_status_0() { return read_register_and_decode("PORT_STATUS_0", port_status_0); }
        esp_err_t read_port_status_1() { return read_register_and_decode("PORT_STATUS_1", port_status_1); }
        esp_err_t read_typec_monitoring_status_0() { return read_register_and_decode("TYPEC_MONITORING_STATUS_0", typec_monitoring_status_0); }
        esp_err_t read_typec_monitoring_status_1() { return read_register_and_decode("TYPEC_MONITORING_STATUS_1", typec_monitoring_status_1); }
        esp_err_t read_cc_status() { return read_register_and_decode("CC_STATUS", cc_status); }
        esp_err_t read_cc_hw_fault_status_0() { return read_register_and_decode("CC_HW_FAULT_STATUS_0", cc_hw_fault_0); }
        esp_err_t read_cc_hw_fault_status_1() { return read_register_and_decode("CC_HW_FAULT_STATUS_1", cc_hw_fault_1); }
        esp_err_t read_pd_typec_status() { return read_register_and_decode("PD_TYPEC_STATUS", pd_typec_status); }
        esp_err_t read_typec_status() { return read_register_and_decode("TYPEC_STATUS", typec_status); }
        esp_err_t read_prt_status() { return read_register_and_decode("PRT_STATUS", prt_status); }

        esp_err_t get_status();

        void log() const;
        std::string to_json() const;

    private:
        inline static const char *TAG = "STUSB4500-STATUS";
    };

} // namespace stusb4500
