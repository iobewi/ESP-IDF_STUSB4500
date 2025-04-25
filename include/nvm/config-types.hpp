#pragma once

#include <cstdint>

namespace stusb4500 {
    // NVM Config
    // Fonctionnalité du GPIO (Bank1 @ 0xC8)
    enum class GPIOFunction : uint8_t {
        ErrorRecovery = 0b00,
        Debug         = 0b01,
        SinkPower     = 0b10,
        SWCtrl        = 0b11
    };

    constexpr const char* GPIOFunction_to_string(GPIOFunction function) {
        switch (function) {
            case GPIOFunction::ErrorRecovery:   return "Hardware fault detection";
            case GPIOFunction::Debug:           return "Debug accessory detection";
            case GPIOFunction::SinkPower:       return "Indicates USB Type-C current capability advertised by the source";
            case GPIOFunction::SWCtrl:          return "Software controlled GPIO.";
        }
        return "Unknown";
    }

    enum class PowerOkConfig : uint8_t {
        CONFIG_1 = 0b00,     // Configuration 1
        NOT_APPLICABLE = 0b01, // Non applicable
        CONFIG_2 = 0b10,     // Configuration 2 (default)
        CONFIG_3 = 0b11      // Configuration 3
    };

    constexpr const char* PowerOkConfig_to_string(PowerOkConfig cfg) {
        switch (cfg) {
            case PowerOkConfig::CONFIG_1:        return "Configuration 1";
            case PowerOkConfig::NOT_APPLICABLE:  return "Not applicable";
            case PowerOkConfig::CONFIG_2:        return "Configuration 2 (default)";
            case PowerOkConfig::CONFIG_3:        return "Configuration 3";
        }
        return "Unknown";
    }

    inline const char* to_string(GPIOFunction f) { return GPIOFunction_to_string(f); }
    inline const char* to_string(PowerOkConfig cfg) { return PowerOkConfig_to_string(cfg); }

    enum class SinkPDOCurrent : uint8_t {
        FLEX    = 0b0000,
        MA_500  = 0b0001,
        MA_750  = 0b0010,
        MA_1000 = 0b0011,
        MA_1250 = 0b0100,
        MA_1500 = 0b0101,
        MA_1750 = 0b0110,
        MA_2000 = 0b0111,
        MA_2250 = 0b1000,
        MA_2500 = 0b1001,
        MA_2750 = 0b1010,
        MA_3000 = 0b1011,
        MA_3500 = 0b1100,
        MA_4000 = 0b1101,
        MA_4500 = 0b1110,
        MA_5000 = 0b1111,
    };

    class PDOCurrent {
        public:
            explicit constexpr PDOCurrent(SinkPDOCurrent code) : code_(code) {}
        
            static constexpr PDOCurrent from_ma(uint16_t ma) {
                if (ma == 0) return PDOCurrent(SinkPDOCurrent::FLEX);
        
                // Plage 500 mA à 3000 mA (pas 250 mA)
                if (ma >= 500 && ma <= 3000 && (ma % 250) == 0) {
                    uint8_t step = (ma - 500) / 250 + 1;
                    return PDOCurrent(static_cast<SinkPDOCurrent>(step));
                }
        
                // Plage 3500 à 5000 mA (pas 500 mA)
                if (ma >= 3500 && ma <= 5000 && (ma % 500) == 0) {
                    uint8_t step = ((ma - 3000) / 500) + 12;
                    return PDOCurrent(static_cast<SinkPDOCurrent>(step));
                }
        
                // fallback
                return PDOCurrent(SinkPDOCurrent::FLEX);
            }
        
            constexpr uint16_t to_ma() const {
                uint8_t val = static_cast<uint8_t>(code_);
                if (code_ == SinkPDOCurrent::FLEX) return 0;
        
                if (val >= 1 && val <= 11) {
                    return 500 + (val - 1) * 250;
                } else if (val >= 12 && val <= 15) {
                    return 3000 + (val - 11) * 500;
                }
                return 0;
            }
        
            constexpr SinkPDOCurrent code() const { return code_; }
        
        private:
            SinkPDOCurrent code_;
        };
        

    class PDOVoltage {
        public:
            explicit constexpr PDOVoltage(uint16_t step) : step_(step) {}
        
            constexpr uint16_t step() const { return step_; }
            constexpr uint16_t to_mv() const { return step_ * 50; }
        
            static constexpr PDOVoltage from_mv(uint16_t mv) {
                uint16_t step = mv / 50;
                if (step < 100) step = 100;
                if (step > 400) step = 400;
                return PDOVoltage(step);
            }
        
        private:
            uint16_t step_;
        };

    class PDOShift {
        public:
            // Constructeur explicite à partir d’un pourcentage (1 à 15 %)
            explicit constexpr PDOShift(uint8_t percent)
                : reg_((percent < 1) ? 1 : (percent > 15 ? 15 : percent)) {}
        
            // Retourne le pourcentage
            constexpr uint8_t to_percent() const { return reg_; }
        
            // Création depuis une valeur brute du registre
            static constexpr PDOShift from_percent(uint8_t raw) {
                return PDOShift((raw < 1) ? 1 : (raw > 15 ? 15 : raw));
            }
        
        private:
            uint8_t reg_;
        };
            

    // Configuration d’un PDO (Voltage en mV, courant en mA)
    struct PDOConfig {
        uint16_t voltage_mv; // Tension (mV)
        uint16_t current_ma; // Courant (mA)
        bool enabled;        // PDO actif ou non
    };

    // Seuils de monitoring VBUS (exprimés en %)
    struct VBUSMonitorThresholds {
        uint8_t lower_percent; // SHIFT_VBUS_LLx (0–15)
        uint8_t upper_percent; // SHIFT_VBUS_HLx (0–15)
    };

    struct ConfigSYS {
        // === Bank1 ===
        GPIOFunction gpio_cfg;           // GPIO_CFG[1:0]
        bool vbus_only_above_5v;         // POWER_ONLY_ABOVE_5V
        uint8_t discharge_time_to_0v;   // 0–15, 84 ms/unit
        uint8_t vbus_discharge_time_to_pdo;  // 0–15, 24 ms/unit

        // === Bank3 ===
        uint8_t pdo_count;               // DPM_SNK_PDO_NUMB[1:0]
        bool usb_comm_capable;          // USB_COMM_CAPABLE
        bool unconstrained_power;       // SNK_UNCONS_POWER

        PDOConfig pdo1;                  // PDO1 (5 V typiquement)
        PDOConfig pdo2;                  // PDO2
        PDOConfig pdo3;                  // PDO3

        // === Bank4 : flexibles et options avancées ===
        uint16_t flexible_current_ma;    // en mA (step 10 mA, max 5 A)
        uint16_t pdo2_voltage_mv;        // si flexible
        uint16_t pdo3_voltage_mv;        // si flexible

        VBUSMonitorThresholds pdo1_thresholds;
        VBUSMonitorThresholds pdo2_thresholds;
        VBUSMonitorThresholds pdo3_thresholds;

        PowerOkConfig power_ok_cfg;           // POWER_OK_CFG[1:0] (0, 1, 2, 3)
        bool req_src_current;           // REQ_SRC_CURRENT
    };

} 
