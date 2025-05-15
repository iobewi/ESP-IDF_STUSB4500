#include "status/stusb4500-status_types.hpp"

#include "esp_log.h"

namespace stusb4500
{
    static const char *TAG = "STUSB4500-STATUS";
    inline std::string json_bool(const char *key, bool value)
    {
        return "\"" + std::string(key) + "\": " + (value ? "true" : "false");
    }

    std::string StateStatusRegister::to_string(uint8_t val)
    {
        switch (val)
        {
        case 0x00:
            return "PE_INIT";
        case 0x01:
            return "PE_SOFT_RESET";
        case 0x02:
            return "PE_HARD_RESET";
        case 0x03:
            return "PE_SEND_SOFT_RESET";
        case 0x04:
            return "PE_C_BIST";
        case 0x12:
            return "PE_SNK_STARTUP";
        case 0x13:
            return "PE_SNK_DISCOVERY";
        case 0x14:
            return "PE_SNK_WAIT_FOR_CAPABILITIES";
        case 0x15:
            return "PE_SNK_EVALUATE_CAPABILITIES";
        case 0x16:
            return "PE_SNK_SELECT_CAPABILITIES";
        case 0x17:
            return "PE_SNK_TRANSITION_SINK";
        case 0x18:
            return "PE_SNK_READY";
        case 0x19:
            return "PE_SNK_READY_SENDING";
        case 0x3A:
            return "PE_HARD_RESET_SHUTDOWN";
        case 0x3B:
            return "PE_HARD_RESET_RECOVERY";
        case 0x40:
            return "PE_ERRORRECOVERY";
        default:
            return "UNKNOWN";
        }
    }

    void StateStatusRegister::log() const
    {
        ESP_LOGI(TAG, "PE_FSM: 0x%02X (%s)", raw_, to_string(raw_));
    }

    std::string StateStatusRegister::to_json() const
    {
        char buf[64];
        snprintf(buf, sizeof(buf), "{\"pe_state\": \"%s\"}", to_string(raw_).c_str());
        return std::string(buf);
    }

    AlertStatus1Register::AlertStatus1Reg AlertStatus1Register::get_values() const
    {
        AlertStatus1Reg values = {};
        values.port_status_al = raw_ & (1 << 6);
        values.typec_monitoring_status_al = raw_ & (1 << 5);
        values.cc_hw_fault_status_al = raw_ & (1 << 4);
        values.pd_typec_status_al = raw_ & (1 << 3);
        values.prt_status_al = raw_ & (1 << 1);
        return values;
    }

    void AlertStatus1Register::log() const
    {
        AlertStatus1Reg values = get_values();
        ESP_LOGI(TAG, "ALERT_STATUS_1: PORT_STATUS_AL=%s, TYPEC_MONITORING_STATUS_AL=%s, CC_HW_FAULT_STATUS_AL=%s, PD_TYPEC_STATUS_AL=%s, PRT_STATUS_AL=%s",
                 values.port_status_al ? "YES" : "NO",
                 values.typec_monitoring_status_al ? "YES" : "NO",
                 values.cc_hw_fault_status_al ? "YES" : "NO",
                 values.pd_typec_status_al ? "YES" : "NO",
                 values.prt_status_al ? "YES" : "NO");
    }

    std::string AlertStatus1Register::to_json() const
    {
        AlertStatus1Reg values = get_values();
        return std::string("{") +
               json_bool("port_status_al", values.port_status_al) + "," +
               json_bool("typec_monitoring_status_al", values.typec_monitoring_status_al) + "," +
               json_bool("cc_hw_fault_status_al", values.cc_hw_fault_status_al) + "," +
               json_bool("pd_typec_status_al", values.pd_typec_status_al) + "," +
               json_bool("prt_status_al", values.prt_status_al) +
               "}";
    }

    void PortStatus0Register::log() const
    {
        bool value = get_values();
        ESP_LOGI(TAG, "PORT_STATUS_0: attach_transition=%s", value ? "YES" : "NO");
    }

    std::string PortStatus0Register::to_json() const
    {
        bool value = get_values();
        return std::string("{") +
               json_bool("attach_transition", value) +
               "}";
    }

    PortStatus1Register::PortStatus1Reg PortStatus1Register::get_values() const
    {
        PortStatus1Reg values = {};
        values.raw_attached_device = (raw_ >> 5) & 0x07;
        values.power_mode = raw_ & 0x10;
        values.data_mode = raw_ & 0x08;
        values.attached = raw_ & 0x01;
        return values;
    }

    std::string PortStatus1Register::to_string(uint8_t attached_device)
    {
        switch (attached_device)
        {
        case 0:
            return "None";
        case 1:
            return "Sink";
        case 3:
            return "Debug Accessory";
        default:
            return "Reserved";
        }
    }

    void PortStatus1Register::log() const
    {
        PortStatus1Reg values = get_values();
        ESP_LOGI(TAG, "PORT_STATUS_1: attached_device=%u (%s), power_mode=%s, data_mode=%s, attached=%s",
            values.raw_attached_device, to_string(values.raw_attached_device).c_str(),
            values.power_mode ? "ON" : "OFF", values.data_mode ? "YES" : "NO", values.attached ? "YES" : "NO");
    }

    std::string PortStatus1Register::to_json() const
    {
        PortStatus1Reg values = get_values();
        return std::string("{") +
               "\"attached_device\": {\"value\": " + std::to_string(values.raw_attached_device) +
               ", \"label\": \"" + to_string(values.raw_attached_device) + "\"}," +
               json_bool("power_mode", values.power_mode) + "," +
               json_bool("data_mode", values.data_mode) + "," +
               json_bool("attached", values.attached) +
               "}";
    }

    TypeCMonitoringStatus0Register::TypeCMonitoringStatus0Reg TypeCMonitoringStatus0Register::get_values() const
    {
        TypeCMonitoringStatus0Reg values ={};
        values.vbus_high_ko = raw_ & (1 << 5);
        values.vbus_low_ko = raw_ & (1 << 4);
        values.vbus_ready_trans = raw_ & (1 << 3);
        values.vbus_vsafe0v_trans = raw_ & (1 << 2);
        values.vbus_valid_snk_trans = raw_ & (1 << 1);
        return values;
    }

    void TypeCMonitoringStatus0Register::log() const
    {
        TypeCMonitoringStatus0Reg values = get_values();
        ESP_LOGI(TAG, "TYPEC_MONITORING_STATUS_0: HIGH_KO=%s, LOW_KO=%s, READY_TRANS=%s, VSAFE0V_TRANS=%s, VALID_SNK_TRANS=%s",
                 values.vbus_high_ko ? "YES" : "NO",
                 values.vbus_low_ko ? "YES" : "NO",
                 values.vbus_ready_trans ? "YES" : "NO",
                 values.vbus_vsafe0v_trans ? "YES" : "NO",
                 values.vbus_valid_snk_trans ? "YES" : "NO");
    }

    std::string TypeCMonitoringStatus0Register::to_json() const
    {
        TypeCMonitoringStatus0Reg values = get_values();
        return std::string("{") +
               json_bool("vbus_high_ko", values.vbus_high_ko) + "," +
               json_bool("vbus_low_ko", values.vbus_low_ko) + "," +
               json_bool("vbus_ready_trans", values.vbus_ready_trans) + "," +
               json_bool("vbus_vsafe0v_trans", values.vbus_vsafe0v_trans) + "," +
               json_bool("vbus_valid_snk_trans", values.vbus_valid_snk_trans) +
               "}";
    }

    TypeCMonitoringStatus1Register::TypeCMonitoringStatus1Reg TypeCMonitoringStatus1Register::get_values() const
    {   
        TypeCMonitoringStatus1Reg values ={};
        values.vbus_ready = raw_ & (1 << 3);
        values.vbus_vsafe0v = raw_ & (1 << 2);
        values.vbus_valid_snk = raw_ & (1 << 1);
        return values;
    }

    void TypeCMonitoringStatus1Register::log() const
    {
        TypeCMonitoringStatus1Reg values = get_values();
        ESP_LOGI(TAG, "TYPEC_MONITORING_STATUS_1: READY=%s, VSAFE0V=%s, VALID_SNK=%s",
                 values.vbus_ready ? "YES" : "NO",
                 values.vbus_vsafe0v ? "YES" : "NO",
                 values.vbus_valid_snk ? "YES" : "NO");
    }

    std::string TypeCMonitoringStatus1Register::to_json() const
    {
        TypeCMonitoringStatus1Reg values = get_values();
        return std::string("{") +
               json_bool("vbus_ready", values.vbus_ready) + "," +
               json_bool("vbus_vsafe0v", values.vbus_vsafe0v) + "," +
               json_bool("vbus_valid_snk", values.vbus_valid_snk) +
               "}";
    }


    CCStatusRegister::CCStatusReg CCStatusRegister::get_values() const
    {
        CCStatusReg values = {};
        values.looking_for_connection = raw_ & (1 << 5);
        values.connect_result = raw_ & (1 << 4);
        values.raw_cc2_state = (raw_ >> 2) & 0x03;
        values.raw_cc1_state = raw_ & 0x03;
        return values;
    }

    std::string CCStatusRegister::to_string(uint8_t state)
    {
        switch (state)
        {
        case 1:
            return "SNK.Default";
        case 2:
            return "SNK.Power1.5";
        case 3:
            return "SNK.Power3.0";
        default:
            return "Reserved";
        }
    }

    void CCStatusRegister::log() const
    {
        CCStatusReg values = get_values();
        ESP_LOGI(TAG, "CC_STATUS: LOOKING=%s, CONNECT_RESULT=%s, CC2_STATE=%u (%s), CC1_STATE=%u (%s)",
                 values.looking_for_connection ? "YES" : "NO",
                 values.connect_result ? "PRESENT_RD" : "RESERVED",
                 values.raw_cc2_state, to_string(values.raw_cc2_state).c_str(),
                 values.raw_cc1_state, to_string(values.raw_cc1_state).c_str());
    }

    std::string CCStatusRegister::to_json() const
    {
        CCStatusReg values = get_values();
        return std::string("{") +
               json_bool("looking_for_connection", values.looking_for_connection) + "," +
               json_bool("connect_result", values.connect_result) + "," +
               "\"cc2_state\": {\"value\": " + std::to_string(values.raw_cc2_state) + ", \"label\": \"" + to_string(values.raw_cc2_state) + "\"}," +
               "\"cc1_state\": {\"value\": " + std::to_string(values.raw_cc1_state) + ", \"label\": \"" + to_string(values.raw_cc1_state) + "\"}" +
               "}";
    }

    CCHwFaultStatus0Register::CCHwFaultStatus0Reg CCHwFaultStatus0Register::get_values() const
    {
        CCHwFaultStatus0Reg values = {};
        values.vpu_ovp_fault_trans = raw_ & (1 << 5);
        values.vpu_valid_trans = raw_ & (1 << 4);
        return values;
    }

    void CCHwFaultStatus0Register::log() const
    {
        CCHwFaultStatus0Reg values = get_values();
        ESP_LOGI(TAG, "CC_HW_FAULT_STATUS_0: VPU_OVP_TRANS=%s, VPU_VALID_TRANS=%s",
                 values.vpu_ovp_fault_trans ? "YES" : "NO",
                 values.vpu_valid_trans ? "YES" : "NO");
    }

    std::string CCHwFaultStatus0Register::to_json() const
    {
        CCHwFaultStatus0Reg values = get_values();
        return std::string("{") +
               json_bool("vpu_ovp_fault_trans", values.vpu_ovp_fault_trans) + "," +
               json_bool("vpu_valid_trans", values.vpu_valid_trans) +
               "}";
    }

    CCHwFaultStatus1Register::CCHwFaultStatus1Reg CCHwFaultStatus1Register::get_values() const
    {
        CCHwFaultStatus1Reg values = {};
        values.vpu_ovp_fault = raw_ & (1 << 7);
        values.vpu_valid = raw_ & (1 << 6);
        values.vbus_disch_fault = raw_ & (1 << 4);
        return values;
    }

    void CCHwFaultStatus1Register::log() const
    {
        CCHwFaultStatus1Reg values = get_values();
        ESP_LOGI(TAG, "CC_HW_FAULT_STATUS_1: VPU_OVP=%s, VPU_VALID=%s, VBUS_DISCH_FAULT=%s",
                 values.vpu_ovp_fault ? "YES" : "NO",
                 values.vpu_valid ? "YES" : "NO",
                 values.vbus_disch_fault ? "YES" : "NO");
    }

    std::string CCHwFaultStatus1Register::to_json() const
    {        CCHwFaultStatus1Reg values = get_values();
        return std::string("{") +
               json_bool("vpu_ovp_fault", values.vpu_ovp_fault) + "," +
               json_bool("vpu_valid", values.vpu_valid) + "," +
               json_bool("vbus_disch_fault", values.vbus_disch_fault) +
               "}";
    }

    std::string PDTypeCStatusRegister::to_string(uint8_t value)
    {
        switch (value)
        {
        case 0x00:
            return "Cleared";
        case 0x08:
            return "Hard Reset complete";
        case 0x0E:
            return "Hard Reset received";
        case 0x0F:
            return "Hard Reset send";
        default:
            return "Reserved";
        }
    }

    void PDTypeCStatusRegister::log() const
    {
        ESP_LOGI(TAG, "PD_TYPEC_STATUS: %s",to_string(raw_).c_str());
    }

    std::string PDTypeCStatusRegister::to_json() const
    {
        return std::string("{") +
               "\"handshake\": " + to_string(raw_).c_str() +
               "}";
    }
    TypeCStatusRegister::TypeCStatusReg TypeCStatusRegister::get_values() const
    {
        TypeCStatusReg values = {};
        values.cc_reverse = raw_ & (1 << 7);
        values.raw_typec_fsm_state = raw_ & 0x1F;
        return values;
    }

    std::string TypeCStatusRegister::to_string(uint8_t value)
    {
        switch (value)
        {
        case 0x00:
            return "Unattached Sink";
        case 0x01:
            return "AttachWait Sink";
        case 0x02:
            return "Attached Sink";
        case 0x03:
            return "Debug Accessory Sink";
        case 0x0C:
            return "Try Source";
        case 0x0D:
            return "Unattached Accessory";
        case 0x0E:
            return "AttachWait Accessory";
        case 0x13:
            return "Error Recovery";
        default:
            return "Reserved";
        }
    }

    void TypeCStatusRegister::log() const
    {
        TypeCStatusReg values = get_values();
        ESP_LOGI(TAG, "TYPEC_STATUS: ORIENTATION=%s, FSM_STATE=%u (%s)",
            values.cc_reverse ? "CC2" : "CC1", values.raw_typec_fsm_state, to_string(values.raw_typec_fsm_state).c_str());
    }

    std::string TypeCStatusRegister::to_json() const
    {
        TypeCStatusReg values = get_values();
        return std::string("{") +
               json_bool("cc_reverse", values.cc_reverse) + "," +
               "\"fsm_state\": " + std::to_string(values.raw_typec_fsm_state) +
               "}";
    }
    PRTStatusRegister::PRTStatusReg PRTStatusRegister::get_values() const
    {
        PRTStatusReg values = {};
        values.prt_ibist_received = raw_ & (1 << 4);  // Bit 4
        values.prl_msg_received = raw_ & (1 << 2);    // Bit 2
        values.prl_hw_rst_received = raw_ & (1 << 0); // Bit 0
        return values;
    }

    void PRTStatusRegister::log() const
    {
        PRTStatusReg values = get_values();
        ESP_LOGI(TAG, "PRT_STATUS: IBIST_RECEIVED=%s, MSG_RECEIVED=%s, HW_RST_RECEIVED=%s",
                 values.prt_ibist_received ? "YES" : "NO",
                 values.prl_msg_received ? "YES" : "NO",
                 values.prl_hw_rst_received ? "YES" : "NO");
    }

    std::string PRTStatusRegister::to_json() const
    {
        PRTStatusReg values = get_values();
        return std::string("{") +
               json_bool("prt_ibist_received", values.prt_ibist_received) + "," +
               json_bool("prl_msg_received", values.prl_msg_received) + "," +
               json_bool("prl_hw_rst_received", values.prl_hw_rst_received) +
               "}";
    }

} // namespace stusb4500
