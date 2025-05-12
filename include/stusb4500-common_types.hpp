#pragma once
#include <vector>
#include <cstdint>
#include <string>

namespace stusb4500
{
    enum class FastRoleSwap : uint8_t {
        NotSupported = 0b00,
        DefaultUSB   = 0b01,
        A_1_5        = 0b10,
        A_3_0        = 0b11
    };

    struct VBUSMonitorThresholds
    {
        uint8_t lower_percent;
        uint8_t upper_percent;
    };

    struct PDObjectProfile{
        uint16_t voltage_mv = 5000;
        uint16_t current_ma = 1500 ;
        VBUSMonitorThresholds vbus_monitor {15 , 5};
        bool defined = false;
        
        void log() const;
        std::string to_json() const;
    };
    
    struct PowerProfile
    {
        std::vector<PDObjectProfile> pdos{};
        uint8_t pdo_number = 1;
        uint16_t flex_current_ma = 2200;
        bool usb_comm_capable = false;
        bool dual_role_power = false;
        bool higher_capability = false;
        bool unconstrained_power = false;
        FastRoleSwap frs = FastRoleSwap::NotSupported;

        void decode(uint32_t raw, size_t index);
        uint32_t encode(size_t index) const;

        void log() const;
        std::string to_json() const;
    };

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
};