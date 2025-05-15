#include "status/stusb4500-status.hpp"

#define RETURN_IF_ERROR(x)         \
    do {                           \
        esp_err_t __err_rc = (x);  \
        if (__err_rc != ESP_OK) {  \
            return __err_rc;       \
        }                          \
    } while (0)

namespace stusb4500
{

    esp_err_t STATUS::get_status()
    {
        RETURN_IF_ERROR(read_policy_engine_state());
        RETURN_IF_ERROR(read_port_status_0());
        RETURN_IF_ERROR(read_port_status_1());
        RETURN_IF_ERROR(read_typec_monitoring_status_0());
        RETURN_IF_ERROR(read_typec_monitoring_status_1());
        RETURN_IF_ERROR(read_cc_status());
        RETURN_IF_ERROR(read_cc_hw_fault_status_0());
        RETURN_IF_ERROR(read_cc_hw_fault_status_1());
        RETURN_IF_ERROR(read_pd_typec_status());
        RETURN_IF_ERROR(read_typec_status());
        RETURN_IF_ERROR(read_prt_status());
        return ESP_OK;
    }

    void STATUS::log() const
    {
        policy_engine_state.log();
        port_status_0.log();
        port_status_1.log();
        typec_monitoring_status_0.log();
        typec_monitoring_status_1.log();
        cc_status.log();
        cc_hw_fault_0.log();
        cc_hw_fault_1.log();
        pd_typec_status.log();
        typec_status.log();
        prt_status.log();
    }

    std::string STATUS::to_json() const
    {
        return std::string("{") +
               "\"policy_engine_state\": " + policy_engine_state.to_json() + "," +
               "\"port_status_0\": " + port_status_0.to_json() + "," +
               "\"port_status_1\": " + port_status_1.to_json() + "," +
               "\"typec_monitoring_status_0\": " + typec_monitoring_status_0.to_json() + "," +
               "\"typec_monitoring_status_1\": " + typec_monitoring_status_1.to_json() + "," +
               "\"cc_status\": " + cc_status.to_json() + "," +
               "\"cc_hw_fault_0\": " + cc_hw_fault_0.to_json() + "," +
               "\"cc_hw_fault_1\": " + cc_hw_fault_1.to_json() + "," +
               "\"pd_typec_status\": " + pd_typec_status.to_json() + "," +
               "\"typec_status\": " + typec_status.to_json() + "," +
               "\"prt_status\": " + prt_status.to_json() +
               "}";
    }
} // namespace stusb4500
