#pragma once

#include <string>
#include <cstdint>

#include "esp_log.h"

namespace stusb4500
{
    
    struct StateStatus
    {
        const uint8_t reg_addr = 0x29;
        uint8_t state = 0xFF;

        void decode(uint8_t val);
        static std::string to_string(uint8_t val);

        void log() const;
        std::string to_json() const;
    };

    struct AlertStatus1
    {
        const uint8_t reg_addr = 0x0B;

        bool port_status_al;
        bool typec_monitoring_status_al;
        bool cc_hw_fault_status_al;
        bool pd_typec_status_al;
        bool prt_status_al;

        void decode(uint8_t value);

        void log() const;
        std::string to_json() const;
    };

    struct AlertStatus1Mask
    {
        const uint8_t reg_addr = 0x0C;

        bool port_status_al_mask;
        bool typec_monitoring_status_al_mask;
        bool cc_hw_fault_status_al_mask;
        bool prt_status_al_mask;

        void decode(uint8_t value);
        uint8_t encode() const;

        void log() const;
        std::string to_json() const;
    };

    struct PortStatus0
    {

        const uint8_t reg_addr = 0x0D;

        bool attach_transition;

        void decode(uint8_t value);

        void log() const;
        std::string to_json() const;
    };

    struct PortStatus1
    {

        const uint8_t reg_addr = 0x0E;

        uint8_t raw_attached_device;
        bool power_mode;
        bool data_mode;
        bool attached;

        void decode(uint8_t value);
        static std::string to_string(uint8_t dev);

        void log() const;
        std::string to_json() const;
    };

    struct TypeCMonitoringStatus0
    {

        const uint8_t reg_addr = 0x0F;

        bool vbus_high_ko;
        bool vbus_low_ko;
        bool vbus_ready_trans;
        bool vbus_vsafe0v_trans;
        bool vbus_valid_snk_trans;

        void decode(uint8_t value);

        void log() const;
        std::string to_json() const;
    };

    struct TypeCMonitoringStatus1
    {

        const uint8_t reg_addr = 0x10;

        bool vbus_ready;
        bool vbus_vsafe0v;
        bool vbus_valid_snk;

        void decode(uint8_t value);

        void log() const;
        std::string to_json() const;
    };

    struct CCStatus
    {

        const uint8_t reg_addr = 0x11;

        bool looking_for_connection;
        bool connect_result;
        uint8_t raw_cc2_state;
        uint8_t raw_cc1_state;

        void decode(uint8_t value);
        static std::string to_string(uint8_t state);

        void log() const;
        std::string to_json() const;
    };

    struct CCHwFaultStatus0
    {

        const uint8_t reg_addr = 0x12;

        bool vpu_ovp_fault_trans;
        bool vpu_valid_trans;

        void decode(uint8_t value);

        void log() const;
        std::string to_json() const;
    };

    struct CCHwFaultStatus1
    {

        const uint8_t reg_addr = 0x13;

        bool vpu_ovp_fault;
        bool vpu_valid;
        bool vbus_disch_fault;

        void decode(uint8_t value);

        void log() const;
        std::string to_json() const;
    };

    struct PDTypeCStatus
    {

        const uint8_t reg_addr = 0x14;

        uint8_t handshake;

        void decode(uint8_t value) ;

        void log() const;
        std::string to_json() const;
    };

    struct TypeCStatus
    {

        const uint8_t reg_addr = 0x15;

        bool cc_reverse;
        uint8_t raw_typec_fsm_state;

        void decode(uint8_t value);
        static std::string to_string(uint8_t value);

        void log() const;
        std::string to_json() const;
    };

    struct PRTStatus
    {
        const uint8_t reg_addr = 0x16;

        bool prt_ibist_received;
        bool prl_msg_received;
        bool prl_hw_rst_received;

        void decode(uint8_t value);

        void log() const;
        std::string to_json() const;
    };

}  // namespace stusb4500
