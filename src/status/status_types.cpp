#include "status/stusb4500-status_types.hpp"

#include "esp_log.h"

namespace stusb4500
{
    static const char *TAG = "STUSB4500-STATUS";
    inline std::string json_bool(const char *key, bool value)
    {
        return "\"" + std::string(key) + "\": " + (value ? "true" : "false");
    }

    void StateStatus::decode(uint8_t val)
    {
        state = val;
    }

    std::string StateStatus::to_string(uint8_t val)
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

    void StateStatus::log() const
    {
        ESP_LOGI(TAG, "PE_FSM: 0x%02X (%s)", state, to_string(state));
    }

    std::string StateStatus::to_json() const
    {
        char buf[64];
        snprintf(buf, sizeof(buf), "{\"pe_state\": \"%s\"}", to_string(state).c_str());
        return std::string(buf);
    }

    void AlertStatus1::decode(uint8_t value)
    {
        port_status_al = value & (1 << 6);
        typec_monitoring_status_al = value & (1 << 5);
        cc_hw_fault_status_al = value & (1 << 4);
        pd_typec_status_al = value & (1 << 2);
        prt_status_al = value & (1 << 1);
    }

    void AlertStatus1::log() const
    {
        ESP_LOGI(TAG, "ALERT_STATUS_1: PORT_STATUS_AL=%s, TYPEC_MONITORING_STATUS_AL=%s, CC_HW_FAULT_STATUS_AL=%s, PD_TYPEC_STATUS_AL=%s, PRT_STATUS_AL=%s",
                 port_status_al ? "YES" : "NO",
                 typec_monitoring_status_al ? "YES" : "NO",
                 cc_hw_fault_status_al ? "YES" : "NO",
                 pd_typec_status_al ? "YES" : "NO",
                 prt_status_al ? "YES" : "NO");
    }

    std::string AlertStatus1::to_json() const
    {
        return std::string("{") +
               json_bool("port_status_al", port_status_al) + "," +
               json_bool("typec_monitoring_status_al", typec_monitoring_status_al) + "," +
               json_bool("cc_hw_fault_status_al", cc_hw_fault_status_al) + "," +
               json_bool("pd_typec_status_al", pd_typec_status_al) + "," +
               json_bool("prt_status_al", prt_status_al) +
               "}";
    }

    void AlertStatus1Mask::decode(uint8_t value)
    {
        port_status_al_mask = value & (1 << 6);
        typec_monitoring_status_al_mask = value & (1 << 5);
        cc_hw_fault_status_al_mask = value & (1 << 4);
        prt_status_al_mask = value & (1 << 1);
    }

    uint8_t AlertStatus1Mask::encode() const
    {
        uint8_t value = 0;
        if (port_status_al_mask)
            value |= (1 << 6);
        if (typec_monitoring_status_al_mask)
            value |= (1 << 5);
        if (cc_hw_fault_status_al_mask)
            value |= (1 << 4);
        if (prt_status_al_mask)
            value |= (1 << 1);
        return value;
    }

    void AlertStatus1Mask::log() const
    {
        ESP_LOGI(TAG, "ALERT_STATUS_1_MASK: PORT_STATUS_AL_MASK=%s, TYPEC_MONITORING_STATUS_AL_MASK=%s, CC_HW_FAULT_STATUS_AL_MASK=%s, PRT_STATUS_AL_MASK=%s",
                 port_status_al_mask ? "MASKED" : "UNMASKED",
                 typec_monitoring_status_al_mask ? "MASKED" : "UNMASKED",
                 cc_hw_fault_status_al_mask ? "MASKED" : "UNMASKED",
                 prt_status_al_mask ? "MASKED" : "UNMASKED");
    }

    std::string AlertStatus1Mask::to_json() const
    {
        return std::string("{") +
               json_bool("port_status_al_mask", port_status_al_mask) + "," +
               json_bool("typec_monitoring_status_al_mask", typec_monitoring_status_al_mask) + "," +
               json_bool("cc_hw_fault_status_al_mask", cc_hw_fault_status_al_mask) + "," +
               json_bool("prt_status_al_mask", prt_status_al_mask) +
               "}";
    }

    void PortStatus0::decode(uint8_t value) { attach_transition = value & 0x01; }

    void PortStatus0::log() const
    {
        ESP_LOGI(TAG, "PORT_STATUS_0: attach_transition=%s", attach_transition ? "YES" : "NO");
    }

    std::string PortStatus0::to_json() const
    {
        return std::string("{") +
               json_bool("attach_transition", attach_transition) +
               "}";
    }

    void PortStatus1::decode(uint8_t value)
    {
        raw_attached_device = (value >> 5) & 0x07;
        power_mode = value & 0x10;
        data_mode = value & 0x08;
        attached = value & 0x01;
    }

    std::string PortStatus1::to_string(uint8_t dev)
    {
        switch (dev)
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

    void PortStatus1::log() const
    {
        ESP_LOGI(TAG, "PORT_STATUS_1: attached_device=%u (%s), power_mode=%s, data_mode=%s, attached=%s",
                 raw_attached_device, to_string(raw_attached_device).c_str(),
                 power_mode ? "ON" : "OFF", data_mode ? "YES" : "NO", attached ? "YES" : "NO");
    }

    std::string PortStatus1::to_json() const
    {
        return std::string("{") +
               "\"attached_device\": {\"value\": " + std::to_string(raw_attached_device) +
               ", \"label\": \"" + to_string(raw_attached_device) + "\"}," +
               json_bool("power_mode", power_mode) + "," +
               json_bool("data_mode", data_mode) + "," +
               json_bool("attached", attached) +
               "}";
    }

    void TypeCMonitoringStatus0::decode(uint8_t value)
    {
        vbus_high_ko = value & (1 << 5);
        vbus_low_ko = value & (1 << 4);
        vbus_ready_trans = value & (1 << 3);
        vbus_vsafe0v_trans = value & (1 << 2);
        vbus_valid_snk_trans = value & (1 << 1);
    }

    void TypeCMonitoringStatus0::log() const
    {
        ESP_LOGI(TAG, "TYPEC_MONITORING_STATUS_0: HIGH_KO=%s, LOW_KO=%s, READY_TRANS=%s, VSAFE0V_TRANS=%s, VALID_SNK_TRANS=%s",
                 vbus_high_ko ? "YES" : "NO",
                 vbus_low_ko ? "YES" : "NO",
                 vbus_ready_trans ? "YES" : "NO",
                 vbus_vsafe0v_trans ? "YES" : "NO",
                 vbus_valid_snk_trans ? "YES" : "NO");
    }

    std::string TypeCMonitoringStatus0::to_json() const
    {
        return std::string("{") +
               json_bool("vbus_high_ko", vbus_high_ko) + "," +
               json_bool("vbus_low_ko", vbus_low_ko) + "," +
               json_bool("vbus_ready_trans", vbus_ready_trans) + "," +
               json_bool("vbus_vsafe0v_trans", vbus_vsafe0v_trans) + "," +
               json_bool("vbus_valid_snk_trans", vbus_valid_snk_trans) +
               "}";
    }

    void TypeCMonitoringStatus1::decode(uint8_t value)
    {
        vbus_ready = value & (1 << 3);
        vbus_vsafe0v = value & (1 << 2);
        vbus_valid_snk = value & (1 << 1);
    }

    void TypeCMonitoringStatus1::log() const
    {
        ESP_LOGI(TAG, "TYPEC_MONITORING_STATUS_1: READY=%s, VSAFE0V=%s, VALID_SNK=%s",
                 vbus_ready ? "YES" : "NO",
                 vbus_vsafe0v ? "YES" : "NO",
                 vbus_valid_snk ? "YES" : "NO");
    }

    std::string TypeCMonitoringStatus1::to_json() const
    {
        return std::string("{") +
               json_bool("vbus_ready", vbus_ready) + "," +
               json_bool("vbus_vsafe0v", vbus_vsafe0v) + "," +
               json_bool("vbus_valid_snk", vbus_valid_snk) +
               "}";
    }

    void CCStatus::decode(uint8_t value)
    {
        looking_for_connection = value & (1 << 5);
        connect_result = value & (1 << 4);
        raw_cc2_state = (value >> 2) & 0x03;
        raw_cc1_state = value & 0x03;
    }

    std::string CCStatus::to_string(uint8_t state)
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

    void CCStatus::log() const
    {
        ESP_LOGI(TAG, "CC_STATUS: LOOKING=%s, CONNECT_RESULT=%s, CC2_STATE=%u (%s), CC1_STATE=%u (%s)",
                 looking_for_connection ? "YES" : "NO",
                 connect_result ? "PRESENT_RD" : "RESERVED",
                 raw_cc2_state, to_string(raw_cc2_state).c_str(),
                 raw_cc1_state, to_string(raw_cc1_state).c_str());
    }

    std::string CCStatus::to_json() const
    {
        return std::string("{") +
               json_bool("looking_for_connection", looking_for_connection) + "," +
               json_bool("connect_result", connect_result) + "," +
               "\"cc2_state\": {\"value\": " + std::to_string(raw_cc2_state) + ", \"label\": \"" + to_string(raw_cc2_state) + "\"}," +
               "\"cc1_state\": {\"value\": " + std::to_string(raw_cc1_state) + ", \"label\": \"" + to_string(raw_cc1_state) + "\"}" +
               "}";
    }

    void CCHwFaultStatus0::decode(uint8_t value)
    {
        vpu_ovp_fault_trans = value & (1 << 5);
        vpu_valid_trans = value & (1 << 4);
    }

    void CCHwFaultStatus0::log() const
    {
        ESP_LOGI(TAG, "CC_HW_FAULT_STATUS_0: VPU_OVP_TRANS=%s, VPU_VALID_TRANS=%s",
                 vpu_ovp_fault_trans ? "YES" : "NO",
                 vpu_valid_trans ? "YES" : "NO");
    }

    std::string CCHwFaultStatus0::to_json() const
    {
        return std::string("{") +
               json_bool("vpu_ovp_fault_trans", vpu_ovp_fault_trans) + "," +
               json_bool("vpu_valid_trans", vpu_valid_trans) +
               "}";
    }

    void CCHwFaultStatus1::decode(uint8_t value)
    {
        vpu_ovp_fault = value & (1 << 7);
        vpu_valid = value & (1 << 6);
        vbus_disch_fault = value & (1 << 4);
    }

    void CCHwFaultStatus1::log() const
    {
        ESP_LOGI(TAG, "CC_HW_FAULT_STATUS_1: VPU_OVP=%s, VPU_VALID=%s, VBUS_DISCH_FAULT=%s",
                 vpu_ovp_fault ? "YES" : "NO",
                 vpu_valid ? "YES" : "NO",
                 vbus_disch_fault ? "YES" : "NO");
    }

    std::string CCHwFaultStatus1::to_json() const
    {
        return std::string("{") +
               json_bool("vpu_ovp_fault", vpu_ovp_fault) + "," +
               json_bool("vpu_valid", vpu_valid) + "," +
               json_bool("vbus_disch_fault", vbus_disch_fault) +
               "}";
    }

    void PDTypeCStatus::decode(uint8_t value)
    {
        handshake = value & 0x0F;
    }

    void PDTypeCStatus::log() const
    {
        ESP_LOGI(TAG, "PD_TYPEC_STATUS: HANDSHAKE=0x%02X", handshake);
    }

    std::string PDTypeCStatus::to_json() const
    {
        return std::string("{") +
               "\"handshake\": " + std::to_string(handshake) +
               "}";
    }

    void TypeCStatus::decode(uint8_t value)
    {
        cc_reverse = value & (1 << 7);
        raw_typec_fsm_state = value & 0x1F;
    }

    std::string TypeCStatus::to_string(uint8_t value)
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

    void TypeCStatus::log() const
    {
        ESP_LOGI(TAG, "TYPEC_STATUS: ORIENTATION=%s, FSM_STATE=%u (%s)",
                 cc_reverse ? "CC2" : "CC1", raw_typec_fsm_state, to_string(raw_typec_fsm_state).c_str());
    }

    std::string TypeCStatus::to_json() const
    {
        return std::string("{") +
               json_bool("cc_reverse", cc_reverse) + "," +
               "\"fsm_state\": " + std::to_string(raw_typec_fsm_state) +
               "}";
    }

    void PRTStatus::decode(uint8_t value)
    {
        prt_ibist_received = value & (1 << 4);  // Bit 4
        prl_msg_received = value & (1 << 2);    // Bit 2
        prl_hw_rst_received = value & (1 << 0); // Bit 0
    }

    void PRTStatus::log() const
    {
        ESP_LOGI(TAG, "PRT_STATUS: IBIST_RECEIVED=%s, MSG_RECEIVED=%s, HW_RST_RECEIVED=%s",
                 prt_ibist_received ? "YES" : "NO",
                 prl_msg_received ? "YES" : "NO",
                 prl_hw_rst_received ? "YES" : "NO");
    }

    std::string PRTStatus::to_json() const
    {
        return std::string("{") +
               json_bool("prt_ibist_received", prt_ibist_received) + "," +
               json_bool("prl_msg_received", prl_msg_received) + "," +
               json_bool("prl_hw_rst_received", prl_hw_rst_received) +
               "}";
    }

} // namespace stusb4500
