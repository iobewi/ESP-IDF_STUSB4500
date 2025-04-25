#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <sstream>

#include "esp_log.h"

#include "drivers.hpp"

namespace stusb4500
{

    enum class FastRoleSwap : uint8_t {
        NotSupported = 0b00,    // Default
        DefaultUSB   = 0b01,
        A_1_5        = 0b10,
        A_3_0        = 0b11
    };
    
    struct SinkPDOConfig {
        uint16_t voltage_mv;             // ex: 5000, 9000, 15000...
        uint16_t current_ma;             // ex: 500, 1500, 3000...
        bool dual_role_power = false;
        bool higher_capability = false;
        bool unconstrained_power = false;
        bool usb_comm_capable = true;
        FastRoleSwap fast_role_swap = FastRoleSwap::NotSupported;

        void log(uint8_t index = 0) const {
            std::stringstream ss;
            if (index > 0)
                ss << "Sink PDO[" << static_cast<int>(index) << "]: ";
            else
                ss << "Sink PDO: ";
        
            ss << voltage_mv << " mV @ " << current_ma << " mA"
               << " | CommCapable: " << (usb_comm_capable ? "Yes" : "No")
               << ", DualRole: " << (dual_role_power ? "Yes" : "No")
               << ", FRS: " << static_cast<int>(fast_role_swap);
        
            ESP_LOGI("STUSB4500", "%s", ss.str().c_str());
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

        void log() const {
            ESP_LOGI("STUSB4500", "--- USB Connection Status ---");
            ESP_LOGI("STUSB4500", "Attached         : %s", attached ? "Yes" : "No");
            ESP_LOGI("STUSB4500", "Device           : %s", 
                     device == AttachedDevice::SINK ? "Sink" :
                     device == AttachedDevice::DEBUG ? "Debug" :
                     device == AttachedDevice::NONE ? "None" : "Unknown");
            ESP_LOGI("STUSB4500", "Role             : %s", is_ufp ? "UFP (Sink)" : "DFP (Source)");
            ESP_LOGI("STUSB4500", "Sinking Power    : %s", is_sinking_power ? "Yes" : "No");
            ESP_LOGI("STUSB4500", "Looking for Conn : %s", looking_for_connection ? "Yes" : "No");
            ESP_LOGI("STUSB4500", "Connection Result: %s", connect_result ? "Success" : "Pending");
            if (connect_result) {
                ESP_LOGI("STUSB4500", "CC1 Current      : %d", static_cast<int>(cc1_current));
                ESP_LOGI("STUSB4500", "CC2 Current      : %d", static_cast<int>(cc2_current));
            }
        }
        
    };


    struct RDO {
        uint8_t obj_position;     // bits 28-30
        bool giveback;            // bit 27
        bool capability_mismatch; // bit 26
        bool usb_comm_capable;    // bit 25
        bool no_usb_suspend;      // bit 24
        bool unchunked_ext;       // bit 23
        uint16_t operating_ma;    // bits 10-19 (x10)
        uint16_t max_operating_ma;// bits 0-9 (x10)
        uint32_t raw;             // valeur brute

        void log() const {
            ESP_LOGI("STUSB4500", "--- RDO ---");
            ESP_LOGI("STUSB4500", "ObjPos             : %u", obj_position);
            ESP_LOGI("STUSB4500", "GiveBack           : %s", giveback ? "Yes" : "No");
            ESP_LOGI("STUSB4500", "CapabilityMismatch : %s", capability_mismatch ? "Yes" : "No");
            ESP_LOGI("STUSB4500", "USB Comm Capable   : %s", usb_comm_capable ? "Yes" : "No");
            ESP_LOGI("STUSB4500", "No USB Suspend     : %s", no_usb_suspend ? "Yes" : "No");
            ESP_LOGI("STUSB4500", "Unchunked Ext Msg  : %s", unchunked_ext ? "Yes" : "No");
            ESP_LOGI("STUSB4500", "Operating Current  : %u mA", operating_ma);
            ESP_LOGI("STUSB4500", "Max Current        : %u mA", max_operating_ma);
        }
        
    };

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

    struct RXMessage {
        uint16_t header = 0;
        std::vector<uint32_t> data;
    
        // Champ ajouté : type de message (0–15)
        uint8_t message_type() const {
            return header & 0x0F;
        }
    
        // Champ ajouté : nombre d’objets de données (0–7)
        uint8_t num_data_objects() const {
            return (header >> 12) & 0x07;
        }
    
        // Méthodes utilitaires :
        bool is_data_message() const {
            return num_data_objects() > 0;
        }
    
        bool is_control_message() const {
            return num_data_objects() == 0;
        }

        void log() const {
            ESP_LOGI("STUSB4500", "--- RXMessage ---");
            ESP_LOGI("STUSB4500", "Header           : 0x%04X", header);
            ESP_LOGI("STUSB4500", "Message Type     : %u", message_type());
            ESP_LOGI("STUSB4500", "Data Objects     : %u", num_data_objects());
        
            for (size_t i = 0; i < data.size(); ++i) {
                ESP_LOGI("STUSB4500", "  Data[%zu] = 0x%08X", i, data[i]);
            }
        }
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
    
} // namespace stusb4500
