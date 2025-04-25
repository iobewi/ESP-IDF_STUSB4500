// Enums et helpers pour les conversions STUSB4500
#pragma once

#include <cstdint>
#include <array>
#include <vector>
#include <string>
#include <sstream>
#include "interface.hpp"
#include "esp_log.h"
#include "config.hpp"

namespace stusb4500 {

// === Structures globales ===

struct PowerContract {
    bool valid = false;
    uint16_t voltage_mv = 0;
    uint16_t current_ma = 0;
    uint16_t max_current_ma = 0;
    uint8_t pdo_index = 0;

    void log() const {
        if (!valid) {
            ESP_LOGI("STUSB4500", "No active power contract.");
            return;
        }
        ESP_LOGI("STUSB4500", "Power Contract:");
        ESP_LOGI("STUSB4500", "  PDO #%u → %u mV @ %u mA (max %u mA)",
                 pdo_index, voltage_mv, current_ma, max_current_ma);
    }
};

enum class AttachedDevice {
    NONE = 0b000,
    SINK = 0b001,
    DEBUG = 0b011,
    UNKNOWN
};

enum class CurrentCapability {
    DEFAULT = 0b01,
    POWER_1_5 = 0b10,
    POWER_3_0 = 0b11,
    RESERVED
};

struct USBConnectionStatus {
    bool attached;
    AttachedDevice device;
    bool is_sinking_power;
    bool is_ufp;
    bool looking_for_connection;
    bool connect_result;
    CurrentCapability cc1_current;
    CurrentCapability cc2_current;
};

enum class PolicyEngineState : uint8_t {
    INIT                        = 0x00,
    SOFT_RESET                  = 0x01,
    HARD_RESET                  = 0x02,
    SEND_SOFT_RESET             = 0x03,
    C_BIST                      = 0x04,
    SNK_STARTUP                 = 0x05,
    SNK_DISCOVERY               = 0x06,
    SNK_WAIT_FOR_CAPABILITIES  = 0x07,
    SNK_EVALUATE_CAPABILITIES  = 0x08,
    SNK_SELECT_CAPABILITIES    = 0x09,
    SNK_TRANSITION_SINK        = 0x0A,
    SNK_READY                   = 0x0B,
    SNK_READY_SENDING           = 0x0C,
    HARD_RESET_SHUTDOWN         = 0x0D,
    HARD_RESET_RECOVERY         = 0x0E,
    ERROR_RECOVERY              = 0x10
};

enum class AlertBit : uint8_t {
    PORT_STATUS_AL             = (1 << 6),
    TYPEC_MONITORING_STATUS_AL = (1 << 5),
    CC_HW_FAULT_STATUS_AL      = (1 << 4),
    PRT_STATUS_AL              = (1 << 1),
};

// === Structure de configuration pour PDO ===

struct SinkPDOConfig {
    bool dual_role_power = false;
    bool higher_capability = false;
    bool unconstrained_power = false;
    bool usb_comm_capable = false;
    CurrentCapability fast_role_swap = CurrentCapability::DEFAULT;
    uint16_t voltage_mv = 5000;
    uint16_t current_ma = 500;
};

} // namespace stusb4500
