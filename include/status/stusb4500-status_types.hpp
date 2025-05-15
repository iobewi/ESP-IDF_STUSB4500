#pragma once

#include <string>
#include <cstdint>

#include "esp_log.h"

namespace stusb4500
{
    
    class StateStatusRegister
    {
        public:
        const uint8_t reg_addr = 0x29;

        void set_raw(uint8_t raw) { raw_ = raw; }
        uint8_t get_raw() const { return raw_; }
        uint8_t get_value() const{ return raw_; }

        static std::string to_string(uint8_t val);

        void log() const;
        std::string to_json() const;
        private:
            uint8_t raw_ = 0;
    };

    class AlertStatus1Register
    {
        public:
        const uint8_t reg_addr = 0x0B;

        struct AlertStatus1Reg
        {
            bool port_status_al = 0;
            bool typec_monitoring_status_al = 0;
            bool cc_hw_fault_status_al = 0;
            bool pd_typec_status_al = 0;
            bool prt_status_al = 0;
        };

        void set_raw(uint8_t raw) { raw_ = raw; }
        uint8_t get_raw() const { return raw_; }
        AlertStatus1Reg get_values() const;

        void log() const;
        std::string to_json() const;

        private:
            uint8_t raw_ = 0;
    };

    class PortStatus0Register
    {
        public:
        const uint8_t reg_addr = 0x0D;

        void set_raw(uint8_t raw) { raw_ = raw; }
        uint8_t get_raw() const { return raw_; }
        bool get_values() const { return static_cast<bool>(raw_);};

        void log() const;
        std::string to_json() const;

        private:
        uint8_t raw_ = 0;
    };

    class PortStatus1Register
    {
        public:
        const uint8_t reg_addr = 0x0E;

        struct PortStatus1Reg
        {
            uint8_t raw_attached_device = 0;
            bool power_mode = 0;
            bool data_mode = 0;
            bool attached = 0;
        };

        void set_raw(uint8_t raw) { raw_ = raw; }
        uint8_t get_raw() const { return raw_; }
        PortStatus1Reg get_values() const;

        static std::string to_string(uint8_t attached_device);

        void log() const;
        std::string to_json() const;

        private:
        uint8_t raw_ = 0;
    };

    class TypeCMonitoringStatus0Register
    {
        public:
        const uint8_t reg_addr = 0x0F;

        struct TypeCMonitoringStatus0Reg
        {
            bool vbus_high_ko = 0;
            bool vbus_low_ko = 0;
            bool vbus_ready_trans = 0;
            bool vbus_vsafe0v_trans = 0;
            bool vbus_valid_snk_trans = 0;
        };

        void set_raw(uint8_t raw) { raw_ = raw; }
        uint8_t get_raw() const { return raw_; }
        TypeCMonitoringStatus0Reg get_values() const;

        void log() const;
        std::string to_json() const;

        private:
        uint8_t raw_ = 0;
    };

    class TypeCMonitoringStatus1Register
    {
        public:
        const uint8_t reg_addr = 0x10;

        struct TypeCMonitoringStatus1Reg
        {
            bool vbus_ready;
            bool vbus_vsafe0v;
            bool vbus_valid_snk;
        };

        void set_raw(uint8_t raw) { raw_ = raw; }
        uint8_t get_raw() const { return raw_; }
        TypeCMonitoringStatus1Reg get_values() const;

        void log() const;
        std::string to_json() const;

        private:
        uint8_t raw_ = 0;
    };

    class CCStatusRegister
    {
        public:
        const uint8_t reg_addr = 0x11;
        struct CCStatusReg
        {
            bool looking_for_connection;
            bool connect_result;
            uint8_t raw_cc2_state;
            uint8_t raw_cc1_state;
        };

        void set_raw(uint8_t raw) { raw_ = raw; }
        uint8_t get_raw() const { return raw_; }
        CCStatusReg get_values() const;

        static std::string to_string(uint8_t state);

        void log() const;
        std::string to_json() const;
        
        private:
        uint8_t raw_ = 0;
    };

    class CCHwFaultStatus0Register
    {
        public:
        const uint8_t reg_addr = 0x12;
        struct CCHwFaultStatus0Reg
        {
            bool vpu_ovp_fault_trans;
            bool vpu_valid_trans;
        };

        void set_raw(uint8_t raw) { raw_ = raw; }
        uint8_t get_raw() const { return raw_; }
        CCHwFaultStatus0Reg get_values() const;

        void log() const;
        std::string to_json() const;
                
        private:
        uint8_t raw_ = 0;
    };

    class CCHwFaultStatus1Register
    {
        public:
        const uint8_t reg_addr = 0x13;

        struct CCHwFaultStatus1Reg
        {
        bool vpu_ovp_fault;
        bool vpu_valid;
        bool vbus_disch_fault;
        };

        void set_raw(uint8_t raw) { raw_ = raw; }
        uint8_t get_raw() const { return raw_; }
        CCHwFaultStatus1Reg get_values() const;

        void log() const;
        std::string to_json() const;
                        
        private:
        uint8_t raw_ = 0;
    };

    class PDTypeCStatusRegister
    {
        public:
        const uint8_t reg_addr = 0x14;

        void set_raw(uint8_t raw) { raw_ = raw; }
        uint8_t get_raw() const { return raw_; }
        uint8_t get_value() const { return raw_ & 0x0F;};

        static std::string to_string(uint8_t value);

        void log() const;
        std::string to_json() const;
                                
        private:
        uint8_t raw_ = 0;
    };

    class TypeCStatusRegister
    {
        public:
        const uint8_t reg_addr = 0x15;

        struct TypeCStatusReg
        {
        bool cc_reverse;
        uint8_t raw_typec_fsm_state;
        };

        void set_raw(uint8_t raw) { raw_ = raw; }
        uint8_t get_raw() const { return raw_; }
        TypeCStatusReg get_values() const;


        static std::string to_string(uint8_t value);

        void log() const;
        std::string to_json() const;
                                        
        private:
        uint8_t raw_ = 0;
    };

    class PRTStatusRegister
    {
        public:
        const uint8_t reg_addr = 0x16;

        struct PRTStatusReg
        {
        bool prt_ibist_received;
        bool prl_msg_received;
        bool prl_hw_rst_received;
        };

        void set_raw(uint8_t raw) { raw_ = raw; }
        uint8_t get_raw() const { return raw_; }
        PRTStatusReg get_values() const;

        void log() const;
        std::string to_json() const;
                                        
        private:
        uint8_t raw_ = 0;
    };

}  // namespace stusb4500
