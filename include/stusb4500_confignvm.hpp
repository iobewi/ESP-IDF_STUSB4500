#pragma once

#include <cstdint>

namespace stusb4500 {

    // Fonctionnalité du GPIO (Bank1 @ 0xC8)
    enum class GPIOFunction : uint8_t {
        ErrorRecovery = 0b00,
        Debug         = 0b01,
        SinkPower     = 0b10,
        SWCtrl        = 0b11
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

    struct ConfigNVM {
        // === Bank1 ===
        GPIOFunction gpio_cfg;           // GPIO_CFG[1:0]
        bool vbus_only_above_5v;         // POWER_ONLY_ABOVE_5V
        uint8_t vbus_disch_time_to_0v;   // 0–15, 84 ms/unit
        uint8_t vbus_disch_time_to_pdo;  // 0–15, 24 ms/unit

        // === Bank3 ===
        uint8_t pdo_count;               // DPM_SNK_PDO_NUMB[1:0]
        bool usb_comm_capable;          // USB_COMM_CAPABLE
        bool unconstrained_power;       // SNK_UNCONS_POWER

        PDOConfig pdo1;                  // PDO1 (5 V typiquement)
        PDOConfig pdo2;                  // PDO2
        PDOConfig pdo3;                  // PDO3

        // === Bank4 : flexibles et options avancées ===
        bool use_flexible_voltage;       // si true, voltage PDO2/PDO3 = FLEX
        bool use_flexible_current;       // si true, courant = I_SNK_PDO_FLEX
        uint16_t flexible_current_ma;    // en mA (step 10 mA, max 5 A)
        uint16_t pdo2_voltage_mv;        // si flexible
        uint16_t pdo3_voltage_mv;        // si flexible

        VBUSMonitorThresholds pdo1_thresholds;
        VBUSMonitorThresholds pdo2_thresholds;
        VBUSMonitorThresholds pdo3_thresholds;

        uint8_t power_ok_cfg;           // POWER_OK_CFG[1:0] (0, 1, 2, 3)
        bool req_src_current;           // REQ_SRC_CURRENT
    };

} // namespace stusb4500
