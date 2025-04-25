#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <sstream>

#include "esp_log.h"

#include "drivers.hpp"
#include "data-types.hpp"

namespace stusb4500
{
  
    class HELPERS
    {
    public:
        explicit HELPERS(INTERFACE &iface) : iface_(iface) {}

        // 1.2. How to initialize the STUSB4500 properly
        void init_device()
        {
            clear_all_alerts();
            configure_alert_mask();
        }

        
        void clear_all_alerts()
        {
            uint8_t buf[10] = {0};
            iface_.read_register(0x0D, buf, 10); // Read from 0x0D to 0x16
        }
        // ---


        bool negotiation_successful()
        {
            auto rdo = read_rdo();
            return rdo.obj_position != 0;
        }


        PowerContract get_power_contract() {
            auto rdo = read_rdo();
            RXMessage msg = read_rx_message();
        
            if (rdo.obj_position == 0) {
                ESP_LOGW("STUSB4500", "[PowerContract] RDO invalide : obj_position == 0 → négociation échouée ?");
                return {};
            }
        
            if (rdo.obj_position > msg.data.size()) {
                ESP_LOGW("STUSB4500", "[PowerContract] RDO invalide : obj_position (%u) dépasse le nombre de PDO (%zu)",
                         rdo.obj_position, msg.data.size());
                return {};
            }
        
            SinkPDOConfig cfg = parse_sink_pdo(msg.data[rdo.obj_position - 1]);
        
            return PowerContract{
                true,
                cfg.voltage_mv,
                rdo.operating_ma,
                rdo.max_operating_ma,
                rdo.obj_position
            };
        }
        

        void log_rdo(const RDO& rdo) {
            ESP_LOGI("STUSB4500", "Requested RDO:");
            ESP_LOGI("STUSB4500", " - ObjPos             : %u", rdo.obj_position);
            ESP_LOGI("STUSB4500", " - GiveBack           : %s", rdo.giveback ? "Yes" : "No");
            ESP_LOGI("STUSB4500", " - CapabilityMismatch : %s", rdo.capability_mismatch ? "Yes" : "No");
            ESP_LOGI("STUSB4500", " - USB Comm Capable   : %s", rdo.usb_comm_capable ? "Yes" : "No");
            ESP_LOGI("STUSB4500", " - No USB Suspend     : %s", rdo.no_usb_suspend ? "Yes" : "No");
            ESP_LOGI("STUSB4500", " - Unchunked Ext Msg  : %s", rdo.unchunked_ext ? "Yes" : "No");
            ESP_LOGI("STUSB4500", " - Operating Current  : %u mA", rdo.operating_ma);
            ESP_LOGI("STUSB4500", " - Max Current        : %u mA", rdo.max_operating_ma);
        }

        
        PowerContract power_negotiation(const RXMessage& msg) {
            PowerContract contract;

            if (msg.data.empty()) {
                ESP_LOGW("STUSB4500", "Message PD vide, skip.");
                return contract;
            }

            ESP_LOGI("STUSB4500", "Header: 0x%04X, PDOs received: %zu", msg.header, msg.data.size());
        
            for (size_t i = 0; i < msg.data.size(); ++i) {
                auto cfg = parse_sink_pdo(msg.data[i]);
                ESP_LOGI("STUSB4500", "SRC_PDO[%zu]: %u mV @ %u mA", i + 1, cfg.voltage_mv, cfg.current_ma);
            }

            RDO  rdo = read_rdo();
            ESP_LOGI("STUSB4500", "RDO: Pos=%u, %u mA (max %u mA)", rdo.obj_position, rdo.operating_ma, rdo.max_operating_ma);

            if (rdo.obj_position == 0 || rdo.obj_position > msg.data.size()) {
                ESP_LOGW("STUSB4500", "[PowerContract] RDO invalide : obj_position (%u) dépasse le nombre de PDO (%zu)",
                        rdo.obj_position, msg.data.size());
                return contract;
            }
            
            // Contrat actif
            auto pdo = parse_sink_pdo(msg.data[rdo.obj_position - 1]);
            contract.voltage_mv = pdo.voltage_mv;
            contract.current_ma = rdo.operating_ma;
            contract.max_current_ma = rdo.max_operating_ma;
            contract.pdo_index = rdo.obj_position;
            contract.valid = true;

            ESP_LOGI("STUSB4500", "Contrat accepté : %u mV @ %u mA (max %u mA)",
                contract.voltage_mv, rdo.operating_ma, rdo.max_operating_ma);

            
            return contract;
        }
        
        void log_configured_sink_pdos() {
            ESP_LOGI("STUSB4500", "--- Configured Sink PDOs ---");
            for (int i = 0; i < 3; ++i) {
                uint8_t reg = 0x85 + i * 4;
                uint8_t buf[4] = {0};
                if (iface_.read_register(reg, buf, 4) != ESP_OK) {
                    ESP_LOGW("STUSB4500", "Failed to read PDO %d", i + 1);
                    continue;
                }
        
                uint32_t raw = buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);
                auto cfg = parse_sink_pdo(raw);
        
                ESP_LOGI("STUSB4500", "PDO[%d] = %dmV, %dmA, CommCapable: %s, DualRole: %s",
                         i + 1, cfg.voltage_mv, cfg.current_ma,
                         cfg.usb_comm_capable ? "Yes" : "No",
                         cfg.dual_role_power ? "Yes" : "No");
            }
        }

        // 1.3. How to send a USB PD software reset
        bool send_soft_reset()
        {
            uint8_t header = 0x0D;
            if (iface_.write_register(0x51, &header, 1) != ESP_OK)
            {
                ESP_LOGE("STUSB4500", "Failed to write TX_HEADER_LOW");
                return false;
            }

            uint8_t command = 0x26;
            if (iface_.write_register(0x1A, &command, 1) != ESP_OK)
            {
                ESP_LOGE("STUSB4500", "Failed to write PD_COMMAND_CTRL");
                return false;
            }

            ESP_LOGI("STUSB4500", "USB PD Soft Reset command sent.");
            return true;
        }
        // ---

        // --- 1.4. How to fill the PDO registers
        constexpr uint32_t make_sink_pdo(const SinkPDOConfig& cfg) {
            uint32_t pdo = 0;
            pdo |= (0b00 << 30);  // Fixed supply
            pdo |= (cfg.dual_role_power ? 1 : 0) << 29;
            pdo |= (cfg.higher_capability ? 1 : 0) << 28;
            pdo |= (cfg.unconstrained_power ? 1 : 0) << 27;
            pdo |= (cfg.usb_comm_capable ? 1 : 0) << 26;
            pdo |= (static_cast<uint8_t>(cfg.fast_role_swap) & 0x03) << 23;
            pdo |= ((cfg.voltage_mv / 50) & 0x3FF) << 10;
            pdo |= (cfg.current_ma / 10) & 0x3FF;
            return pdo;
        }
        

        bool write_sink_pdo(uint8_t pdo_index, const SinkPDOConfig& cfg) {
            if (pdo_index < 1 || pdo_index > 3) return false;
        
            uint32_t pdo = make_sink_pdo(cfg);
            uint8_t raw[4] = {
                static_cast<uint8_t>(pdo & 0xFF),
                static_cast<uint8_t>((pdo >> 8) & 0xFF),
                static_cast<uint8_t>((pdo >> 16) & 0xFF),
                static_cast<uint8_t>((pdo >> 24) & 0xFF),
            };
        
            uint8_t reg = 0x85 + (pdo_index - 1) * 4;
            return iface_.write_register(reg, raw, 4) == ESP_OK;
        }
        //---

        //--- 1.6 How to force VBUS to 5 V
        void update_valid_pdo_number(uint8_t count) {
            if (count < 1 || count > 3) return;
            iface_.write_register(0x70, &count, 1);
        }

        void force_vbus_5v() {
            update_valid_pdo_number(1);
            send_soft_reset();
        }
        //---
        
        //--- 1.7. How to read USB-C connection STATUS
        USBConnectionStatus read_connection_status() {
            USBConnectionStatus status{};
        
            uint8_t port_status = 0;
            uint8_t cc_status = 0;
        
            iface_.read_register(0x0E, &port_status, 1);  // PORT_STATUS_1
            iface_.read_register(0x11, &cc_status, 1);    // CC_STATUS
        
            status.attached = port_status & (1 << 0);
            status.is_ufp = !(port_status & (1 << 2));
            status.is_sinking_power = !(port_status & (1 << 3));
            uint8_t attached_bits = (port_status >> 5) & 0x07;
            switch (attached_bits) {
                case 0b000: status.device = AttachedDevice::NONE; break;
                case 0b001: status.device = AttachedDevice::SINK; break;
                case 0b011: status.device = AttachedDevice::DEBUG; break;
                default:    status.device = AttachedDevice::UNKNOWN; break;
            }
        
            status.looking_for_connection = (cc_status >> 5) & 0x01;
            status.connect_result = (cc_status >> 4) & 0x01;
        
            if (status.connect_result) {
                status.cc2_current = static_cast<CurrentCapability>((cc_status >> 2) & 0x03);
                status.cc1_current = static_cast<CurrentCapability>(cc_status & 0x03);
            } else {
                status.cc1_current = CurrentCapability::RESERVED;
                status.cc2_current = CurrentCapability::RESERVED;
            }
        
            return status;
        }
        //---

        //--- 1.8. How to read USB PD STATUS
        static RDO decode_rdo(uint32_t raw) {
            RDO r;
            r.raw = raw;
            r.obj_position        = (raw >> 28) & 0x07;
            r.giveback            = (raw >> 27) & 0x01;
            r.capability_mismatch = (raw >> 26) & 0x01;
            r.usb_comm_capable    = (raw >> 25) & 0x01;
            r.no_usb_suspend      = (raw >> 24) & 0x01;
            r.unchunked_ext       = (raw >> 23) & 0x01;
            r.operating_ma        = ((raw >> 10) & 0x3FF) * 10;
            r.max_operating_ma    = (raw & 0x3FF) * 10;
            return r;
        }

        RDO read_rdo()
        {
            RDO r{};
            uint8_t raw[4];
            if (iface_.read_register(0x91, raw, 4) != ESP_OK)
                return r;
        
            uint32_t val = raw[0] | (raw[1] << 8) | (raw[2] << 16) | (raw[3] << 24);
            r.raw = val;
            r.obj_position = (val >> 28) & 0x7;
            r.giveback = (val >> 27) & 0x1;
            r.capability_mismatch = (val >> 26) & 0x1;
            r.usb_comm_capable = (val >> 25) & 0x1;
            r.no_usb_suspend = (val >> 24) & 0x1;
            r.unchunked_ext = (val >> 23) & 0x1;
            r.operating_ma = ((val >> 10) & 0x3FF) * 10;
            r.max_operating_ma = (val & 0x3FF) * 10;
            return r;
        }
        
        // Ajoute une sortie plus complète dans les fonctions d'affichage et d'export JSON :
        // 
        // ESP_LOGI("STUSB4500", "--- RDO Details ---");
        // ESP_LOGI("STUSB4500", "RDO: ObjPos=%u, GiveBack=%d, CapMismatch=%d, CommCap=%d, NoSuspend=%d, Unchunked=%d",
        //          r.obj_position, r.giveback, r.capability_mismatch, r.usb_comm_capable, r.no_usb_suspend, r.unchunked_ext);
        // 
        // Et pour le JSON :
        // 
        // ss << "  \"rdo\": {\n"
        //    << "    \"obj_position\": " << static_cast<int>(r.obj_position) << ",\n"
        //    << "    \"giveback\": " << r.giveback << ",\n"
        //    << "    \"capability_mismatch\": " << r.capability_mismatch << ",\n"
        //    << "    \"usb_comm_capable\": " << r.usb_comm_capable << ",\n"
        //    << "    \"no_usb_suspend\": " << r.no_usb_suspend << ",\n"
        //    << "    \"unchunked_ext\": " << r.unchunked_ext << ",\n"
        //    << "    \"operating_ma\": " << r.operating_ma << ",\n"
        //    << "    \"max_operating_ma\": " << r.max_operating_ma << "\n"
        //    << "  },\n";
        //---

        //--- 1.9. How to access to the PDO from the SOURCE
        bool is_message_received() {
            uint8_t prt_status = 0;
            iface_.read_register(0x16, &prt_status, 1);
            return prt_status & (1 << 0); // Bit 0 : MESSAGE_RECEIVED
        }

        // +++ 1.4. How to decode a received PDO register into SinkPDOConfig
        constexpr SinkPDOConfig parse_sink_pdo(uint32_t raw) {
            SinkPDOConfig cfg;

            cfg.voltage_mv = ((raw >> 10) & 0x3FF) * 50;
            cfg.current_ma = (raw & 0x3FF) * 10;

            cfg.dual_role_power     = raw & (1 << 29);
            cfg.higher_capability   = raw & (1 << 28);
            cfg.unconstrained_power = raw & (1 << 27);
            cfg.usb_comm_capable    = raw & (1 << 26);
            cfg.fast_role_swap      = static_cast<FastRoleSwap>((raw >> 23) & 0x03);

            return cfg;
        }
        //--

        //--- 1.10 How to access the STUSB4500 policy engine state
        PolicyEngineState read_policy_engine_state() {
            uint8_t value = 0;
            iface_.read_register(0x29, &value, 1);
            return static_cast<PolicyEngineState>(value);
        }
        
        void log_policy_engine_state() {
            auto state = read_policy_engine_state();
            ESP_LOGI("STUSB4500", "PE_FSM: 0x%02X (%s)", static_cast<uint8_t>(state), policy_engine_state_to_str(state));
        }
        
        const char* policy_engine_state_to_str(PolicyEngineState state) {
            switch (state) {
                case PolicyEngineState::INIT: return "INIT";
                case PolicyEngineState::SOFT_RESET: return "SOFT_RESET";
                case PolicyEngineState::HARD_RESET: return "HARD_RESET";
                case PolicyEngineState::SEND_SOFT_RESET: return "SEND_SOFT_RESET";
                case PolicyEngineState::C_BIST: return "C_BIST";
                case PolicyEngineState::SNK_STARTUP: return "SNK_STARTUP";
                case PolicyEngineState::SNK_DISCOVERY: return "SNK_DISCOVERY";
                case PolicyEngineState::SNK_WAIT_FOR_CAPABILITIES: return "WAIT_FOR_CAPABILITIES";
                case PolicyEngineState::SNK_EVALUATE_CAPABILITIES: return "EVALUATE_CAPABILITIES";
                case PolicyEngineState::SNK_SELECT_CAPABILITIES: return "SELECT_CAPABILITIES";
                case PolicyEngineState::SNK_TRANSITION_SINK: return "TRANSITION_SINK";
                case PolicyEngineState::SNK_READY: return "SNK_READY";
                case PolicyEngineState::SNK_READY_SENDING: return "READY_SENDING";
                case PolicyEngineState::HARD_RESET_SHUTDOWN: return "RESET_SHUTDOWN";
                case PolicyEngineState::HARD_RESET_RECOVERY: return "RESET_RECOVERY";
                case PolicyEngineState::ERROR_RECOVERY: return "ERROR_RECOVERY";
                default: return "UNKNOWN";
            }
        }
        //---

        //--- 1.11. How to manage alerts

        uint8_t read_alert_status() {
            uint8_t val = 0;
            iface_.read_register(0x0B, &val, 1);
            return val;
        }

        class PRTStatus {
            public:
                explicit PRTStatus(uint8_t raw = 0) : raw_(raw) {}
            
                bool msg_received() const        { return raw_ & (1 << 2); }  // Bit 2
                bool hard_reset_received() const { return raw_ & (1 << 0); }  // Bit 0
            
                uint8_t raw() const { return raw_; }
            
                void log() const {
                    ESP_LOGI("STUSB4500", "[PRT_STATUS] raw = 0x%02X", raw_);
                    ESP_LOGI("STUSB4500", " - MSG_RECEIVED       : %s", msg_received() ? "YES" : "NO");
                    ESP_LOGI("STUSB4500", " - HARD_RESET_RECEIVED: %s", hard_reset_received() ? "YES" : "NO");
                }
            
            private:
                uint8_t raw_;
            };
            
            PRTStatus read_prt_status() {
                uint8_t reg = 0;
                iface_.read_register(0x16, &reg, 1);
                return PRTStatus(reg);
            }
            
            RXMessage read_rx_message() {
                RXMessage msg;
                uint8_t hdr[2];
                if (iface_.read_register(0x31, hdr, 2) != ESP_OK)
                    return msg;
                msg.header = hdr[0] | (hdr[1] << 8);
            
                uint8_t obj_count = (msg.header >> 12) & 0x07;
                for (uint8_t i = 0; i < obj_count; ++i)
                {
                    uint8_t buf[4];
                    if (iface_.read_register(0x33 + i * 4, buf, 4) != ESP_OK) 
                        break;
                    uint32_t obj = buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);
                    msg.data.push_back(obj);
                }
                return msg;
            }

        

        //---

      
        void log_connection_status() {
            auto status = read_connection_status();
        
            ESP_LOGI("STUSB4500", "=== USB-C Connection Status ===");
            ESP_LOGI("STUSB4500", "Attached           : %s", status.attached ? "Yes" : "No");
            ESP_LOGI("STUSB4500", "Device Type        : %s",
                     (status.device == AttachedDevice::NONE ? "None" :
                      status.device == AttachedDevice::SINK ? "Sink" :
                      status.device == AttachedDevice::DEBUG ? "Debug" : "Unknown"));
            ESP_LOGI("STUSB4500", "Role (UFP/DFP)     : %s", status.is_ufp ? "UFP (Device)" : "DFP (Host)");
            ESP_LOGI("STUSB4500", "Sinking Power      : %s", status.is_sinking_power ? "Yes" : "No");
            ESP_LOGI("STUSB4500", "Looking for Conn.  : %s", status.looking_for_connection ? "Yes" : "No");
            ESP_LOGI("STUSB4500", "Connection Result  : %s", status.connect_result ? "Success" : "Pending");
        
            if (status.connect_result) {
                ESP_LOGI("STUSB4500", "CC1 Current  : %d", static_cast<int>(status.cc1_current));
                ESP_LOGI("STUSB4500", "CC2 Current  : %d", static_cast<int>(status.cc2_current));
            }
        }
        
        
        void log_device_status() {
            ESP_LOGI("STUSB4500", "===== STUSB4500 Status =====");
        
            // 1. État de connexion USB
            auto status = read_connection_status();
            ESP_LOGI("STUSB4500", "Attached     : %s", status.attached ? "YES" : "NO");
            ESP_LOGI("STUSB4500", "Device       : %s",
                     status.device == AttachedDevice::SINK ? "Sink" :
                     status.device == AttachedDevice::DEBUG ? "Debug" :
                     status.device == AttachedDevice::NONE ? "None" : "Unknown");
            ESP_LOGI("STUSB4500", "Role         : %s", status.is_ufp ? "UFP (Sink)" : "DFP (Source)");
            ESP_LOGI("STUSB4500", "Sinking Power: %s", status.is_sinking_power ? "YES" : "NO");
        
            // 2. Courant USB (CC)
            ESP_LOGI("STUSB4500", "CC1 Current  : %d", static_cast<int>(status.cc1_current));
            ESP_LOGI("STUSB4500", "CC2 Current  : %d", static_cast<int>(status.cc2_current));
        
            // 3. Négociation PD
            auto rdo = read_rdo();
            if (rdo.obj_position != 0) {
                ESP_LOGI("STUSB4500", "RDO Position : %u", rdo.obj_position);
                ESP_LOGI("STUSB4500", "Current      : %u mA", rdo.operating_ma);
                ESP_LOGI("STUSB4500", "Max Current  : %u mA", rdo.max_operating_ma);
                ESP_LOGI("STUSB4500", "USB Comm     : %s", rdo.usb_comm_capable ? "YES" : "NO");
                ESP_LOGI("STUSB4500", "No Suspend   : %s", rdo.no_usb_suspend ? "YES" : "NO");
            } else {
                ESP_LOGW("STUSB4500", "No active PDO (negotiation failed)");
            }
        
            // 4. PE (Policy Engine) State
            auto pe_state = read_policy_engine_state();
            ESP_LOGI("STUSB4500", "PE FSM       : %s", policy_engine_state_to_str(pe_state));

            // 5.  Lire les alertes
            uint8_t alert = read_alert_status();
            ESP_LOGI("STUSB4500", "ALERT_STATUS_1     : 0x%02X", alert);

            if (alert & static_cast<uint8_t>(AlertBit::PORT_STATUS_AL))
                ESP_LOGW("STUSB4500", "Alert: PORT_STATUS_AL");
            if (alert & static_cast<uint8_t>(AlertBit::TYPEC_MONITORING_STATUS_AL))
                ESP_LOGW("STUSB4500", "Alert: TYPEC_MONITORING_STATUS_AL");
            if (alert & static_cast<uint8_t>(AlertBit::CC_HW_FAULT_STATUS_AL))
                ESP_LOGW("STUSB4500", "Alert: CC_HW_FAULT_STATUS_AL");
            if (alert & static_cast<uint8_t>(AlertBit::PRT_STATUS_AL))
                ESP_LOGW("STUSB4500", "Alert: PRT_STATUS_AL");

            ESP_LOGI("STUSB4500", "=====================================");
        }
        
    private:
        INTERFACE &iface_;
        void configure_alert_mask()
        {
            uint8_t mask = (1 << 1) | (1 << 5) | (1 << 6); // PRT_STATUS, MONITORING_STATUS, CONNECTION_STATUS
            iface_.write_register(0x0C, &mask, 1);
        }
    };

} // namespace stusb4500
