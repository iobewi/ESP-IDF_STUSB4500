#include "nvm/helpers.hpp"
#include "esp_log.h"
#include <cstring>
#include <cstdio>

namespace stusb4500 {

    void decode_all(NVMData& nvm, const uint8_t* buffer) {
        nvm.bank1.decode(&buffer[8]);
        nvm.bank3.decode(&buffer[24]);
        nvm.bank4.decode(&buffer[32]);
    }

    void encode_all(const NVMData& nvm, uint8_t* buffer) {
        std::memcpy(buffer, NVMData::default_nvm_map.data(), NVM_SIZE);
        nvm.bank1.encode(&buffer[8]);
        nvm.bank3.encode(&buffer[24]);
        nvm.bank4.encode(&buffer[32]);
    }

    std::array<uint8_t, NVM_SIZE> to_array(const NVMData& nvm) {
        std::array<uint8_t, NVM_SIZE> result = NVMData::default_nvm_map;
        encode_all(nvm, result.data());
        return result;
    }

    bool equals(const NVMData& nvm, const std::array<uint8_t, NVM_SIZE>& other) {
        return to_array(nvm) == other;
    }

    std::vector<std::tuple<size_t, uint8_t, uint8_t>> diff(const NVMData& nvm, const std::array<uint8_t, NVM_SIZE>& other) {
        std::vector<std::tuple<size_t, uint8_t, uint8_t>> differences;
        auto current = to_array(nvm);
        for (size_t i = 0; i < current.size(); ++i) {
            if (current[i] != other[i]) {
                differences.emplace_back(i, other[i], current[i]);
            }
        }
        return differences;
    }

    void print_diff(const NVMData& nvm, const std::array<uint8_t, NVM_SIZE>& other) {
        auto diffs = diff(nvm, other);
        for (const auto& [i, before, after] : diffs) {
            ESP_LOGW("STUSB4500NVM",
                     "Offset 0x%02X: 0x%02X -> 0x%02X",
                     static_cast<int>(i),
                     static_cast<int>(before),
                     static_cast<int>(after));
        }

        if (diffs.empty()) {
            ESP_LOGW("STUSB4500NVM", "Aucune différence détectée dans la NVM.");
        }
    }

    ConfigSYS getconfig(const NVMData& nvm) {
        ConfigSYS config;

        config.gpio_cfg = static_cast<GPIOFunction>(nvm.bank1.gpio_cfg);
        config.vbus_only_above_5v = static_cast<bool>(nvm.bank1.vbus_dchg_mask);
        config.vbus_discharge_time_to_pdo = nvm.bank1.vbus_disch_time_to_pdo;
        config.discharge_time_to_0v = nvm.bank1.discharge_time_to_0v;

        config.unconstrained_power = static_cast<bool>(nvm.bank3.snk_uncons_power);
        config.pdo_count = nvm.bank3.dpm_snk_pdo_numb;
        config.usb_comm_capable = static_cast<bool>(nvm.bank3.usb_comm_capable);

        config.pdo1 = {
            .voltage_mv = 5000,
            .current_ma = PDOCurrent(static_cast<SinkPDOCurrent>(nvm.bank3.lut_snk_pdo1_i)).to_ma(),
            .enabled = true
        };
        config.pdo1_thresholds = {
            .lower_percent = PDOShift(nvm.bank3.snk_ll_pdo1).to_percent(),
            .upper_percent = PDOShift(nvm.bank3.snk_hl_pdo1).to_percent(),
        };

        config.pdo2 = {
            .voltage_mv = PDOVoltage(nvm.bank4.snk_pdo_flex1_v).to_mv(),
            .current_ma = PDOCurrent(static_cast<SinkPDOCurrent>(nvm.bank3.lut_snk_pdo2_i)).to_ma(),
            .enabled = (config.pdo_count >= 2)
        };
        config.pdo2_thresholds = {
            .lower_percent = PDOShift(nvm.bank3.snk_ll_pdo2).to_percent(),
            .upper_percent = PDOShift(nvm.bank3.snk_hl_pdo2).to_percent(),
        };

        config.pdo3 = {
            .voltage_mv = PDOVoltage(nvm.bank4.snk_pdo_flex2_v).to_mv(),
            .current_ma = PDOCurrent(static_cast<SinkPDOCurrent>(nvm.bank3.lut_snk_pdo3_i)).to_ma(),
            .enabled = (config.pdo_count >= 3)
        };
        config.pdo3_thresholds = {
            .lower_percent = PDOShift(nvm.bank3.snk_ll_pdo3).to_percent(),
            .upper_percent = PDOShift(nvm.bank3.snk_hl_pdo3).to_percent(),
        };

        config.flexible_current_ma = PDOCurrent(static_cast<SinkPDOCurrent>(nvm.bank4.snk_pdo_flex_i)).to_ma();
        config.power_ok_cfg = static_cast<PowerOkConfig>(nvm.bank4.power_ok_cfg);
        config.req_src_current = nvm.bank4.req_src_current;

        return config;
    }

    void apply_config(NVMData& nvm, const ConfigSYS& config) {
        nvm.bank1.gpio_cfg = static_cast<uint8_t>(config.gpio_cfg);
        nvm.bank1.vbus_dchg_mask = config.vbus_only_above_5v;
        nvm.bank1.vbus_disch_time_to_pdo = config.vbus_discharge_time_to_pdo;
        nvm.bank1.discharge_time_to_0v = config.discharge_time_to_0v;

        nvm.bank3.snk_uncons_power = config.unconstrained_power;
        nvm.bank3.dpm_snk_pdo_numb = config.pdo_count;
        nvm.bank3.usb_comm_capable = config.usb_comm_capable;

        nvm.bank3.lut_snk_pdo1_i = static_cast<uint8_t>(PDOCurrent::from_ma(config.pdo1.current_ma).code());
        nvm.bank3.snk_ll_pdo1 = PDOShift::from_percent(config.pdo1_thresholds.lower_percent).to_percent();
        nvm.bank3.snk_hl_pdo1 = PDOShift::from_percent(config.pdo1_thresholds.upper_percent).to_percent();

        nvm.bank3.lut_snk_pdo2_i = static_cast<uint8_t>(PDOCurrent::from_ma(config.pdo2.current_ma).code());
        nvm.bank3.snk_ll_pdo2 = PDOShift::from_percent(config.pdo2_thresholds.lower_percent).to_percent();
        nvm.bank3.snk_hl_pdo2 = PDOShift::from_percent(config.pdo2_thresholds.upper_percent).to_percent();

        nvm.bank3.lut_snk_pdo3_i = static_cast<uint8_t>(PDOCurrent::from_ma(config.pdo3.current_ma).code());
        nvm.bank3.snk_ll_pdo3 = PDOShift::from_percent(config.pdo3_thresholds.lower_percent).to_percent();
        nvm.bank3.snk_hl_pdo3 = PDOShift::from_percent(config.pdo3_thresholds.upper_percent).to_percent();

        nvm.bank4.snk_pdo_flex1_v = PDOVoltage::from_mv(config.pdo2.voltage_mv).step();
        nvm.bank4.snk_pdo_flex2_v = PDOVoltage::from_mv(config.pdo3.voltage_mv).step();
        nvm.bank4.snk_pdo_flex_i = static_cast<uint8_t>(PDOCurrent::from_ma(config.flexible_current_ma).code());

        nvm.bank4.power_ok_cfg = static_cast<uint8_t>(config.power_ok_cfg);
        nvm.bank4.req_src_current = config.req_src_current;
    }

    void dump_config(const ConfigSYS& config) {
        printf("===== STUSB4500 Configuration Dump =====\n");
        printf("[Bank1]\n");
        printf("  GPIO Function:         %s\n", GPIOFunction_to_string(config.gpio_cfg));
        printf("  VBUS > 5V only:        %s\n", config.vbus_only_above_5v ? "Yes" : "No");
        printf("  Discharge to PDO:      %u x 24ms\n", config.vbus_discharge_time_to_pdo);
        printf("  Discharge to 0V:       %u x 84ms\n", config.discharge_time_to_0v);
        printf("[Bank3]\n");
        printf("  PDO Count:             %u\n", config.pdo_count);
        printf("  USB Communication:     %s\n", config.usb_comm_capable ? "Enabled" : "Disabled");
        printf("  Unconstrained Power:   %s\n", config.unconstrained_power ? "Yes" : "No");

        for (int i = 1; i <= config.pdo_count && i <= 3; ++i) {
            const PDOConfig* pdo = nullptr;
            const VBUSMonitorThresholds* thresholds = nullptr;
            switch (i) {
                case 1: pdo = &config.pdo1; thresholds = &config.pdo1_thresholds; break;
                case 2: pdo = &config.pdo2; thresholds = &config.pdo2_thresholds; break;
                case 3: pdo = &config.pdo3; thresholds = &config.pdo3_thresholds; break;
            }

            if (pdo) {
                printf("  PDO%d: Voltage = %u mV, Current = %u mA, Enabled = %s\n",
                       i, pdo->voltage_mv, pdo->current_ma, pdo->enabled ? "Yes" : "No");
                printf("        Thresholds: LL = %u%%, HL = %u%%\n",
                       thresholds->lower_percent, thresholds->upper_percent);
            }
        }

        printf("[Bank4]\n");
        printf("  Flexible PDO Current:  %u mA\n", config.flexible_current_ma);
        printf("  PDO2 Flex Voltage:     %u mV\n", config.pdo2_voltage_mv);
        printf("  PDO3 Flex Voltage:     %u mV\n", config.pdo3_voltage_mv);
        printf("  Power OK Config:       %s\n", PowerOkConfig_to_string(config.power_ok_cfg));
        printf("  Req Src Current:       %s\n", config.req_src_current ? "Yes" : "No");
        printf("========================================\n");
    }

}
