#pragma once
#include <cstdint>

#include "stusb4500-common_types.hpp"

namespace stusb4500
{
    class AlertStatus1MaskRegister
    {
        public:
        uint8_t reg_addr = 0x0C;

        struct AlertStatus1MaskReg
        {
            bool port_status_al_mask = 1;
            bool typec_monitoring_status_al_mask = 1;
            bool cc_hw_fault_status_al_mask = 1;
            bool prt_status_al_mask = 1;
        };

        void set_raw(uint8_t raw) { raw_ = raw; }
        uint8_t get_raw() const { return raw_; }
        AlertStatus1MaskReg get_values() const;

        void log() const;
        std::string to_json() const;

        private:
        uint8_t raw_ = 0;
    };
    
    class ConfigParams
    {
    public:
        ConfigParams();

        enum class PowerOkConfig : uint8_t
        {
            CONFIG_1 = 0b00,
            NOT_APPLICABLE = 0b01,
            CONFIG_2 = 0b10,
            CONFIG_3 = 0b11
        };

        enum class GPIOFunction : uint8_t
        {
            SWCtrl = 0b00,
            ErrorRecovery = 0b01,
            Debug = 0b10,
            SinkPower = 0b11,
        };

        struct DischargeSettings
        {
            uint8_t time_to_0v = 9;
            uint8_t time_to_pdo = 12;
            bool disable = false;
        };

        PowerProfile power_;
        DischargeSettings discharge_ = {};
        GPIOFunction gpio_function = GPIOFunction::ErrorRecovery;
        PowerOkConfig power_ok = PowerOkConfig::CONFIG_2;
        bool req_src_current = false;
        bool power_only_5v = false;
        AlertStatus1MaskRegister alert_mask = {};

        void log() const;
        std::string to_json() const;

    private:
        inline static const char *TAG = "STUSB4500-CONFIG_PARAMS";
        
        static std::string to_string(GPIOFunction func);
        static std::string to_string(PowerOkConfig config);
    };
}