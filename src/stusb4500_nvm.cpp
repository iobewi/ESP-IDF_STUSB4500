#include "stusb4500_nvm.hpp"
#include "stusb4500_regs.hpp"
#include "esp_log.h"
#include "esp_rom_sys.h"

using namespace stusb4500;

static const char *TAG = "STUSB4500NVM";

STUSB4500NVM::STUSB4500NVM(I2CDevice& device)
    : i2c(device)
{

}

esp_err_t STUSB4500NVM::read_nvm(NVMData& nvm)
{
    esp_err_t err;
    uint8_t val;

    /** 2.2.1.1 – NVM Accessibility */
    val = FTP_CUST_PASSWORD;
    if ((err = i2c.write(REG_FTP_KEY, &val, 1)) != ESP_OK) return err;

    /** 2.2.1.2 – NVM Power-up Sequence */
    val = FTP_CUST_RESET;
    if ((err = i2c.write(REG_FTP_CTRL_0, &val, 1)) != ESP_OK) return err;
    esp_rom_delay_us(10); // >2µs
    val = FTP_CUST_RUN;
    if ((err = i2c.write(REG_FTP_CTRL_0, &val, 1)) != ESP_OK) return err;

    /** Lecture des 5 secteurs de 8 octets */
    std::array<uint8_t, 5 * NVM_SECTOR_SIZE> raw_nvm;

    for (uint8_t sector = 0; sector < 5; ++sector)
    {
        val = static_cast<uint8_t>(FtpOpcode::READ);
        if ((err = i2c.write(REG_FTP_CTRL_1, &val, 1)) != ESP_OK) return err;

        val = FTP_CUST_REQ | (FTP_CUST_SECT + sector);
        if ((err = i2c.write(REG_FTP_CTRL_0, &val, 1)) != ESP_OK) return err;

        esp_rom_delay_us(1000); // délai > 1ms

        if ((err = i2c.read(REG_RW_BUFFER, &raw_nvm[sector * NVM_SECTOR_SIZE], NVM_SECTOR_SIZE)) != ESP_OK)
            return err;
    }

    /** Sortie du mode test */
    if ((err = exit_test_mode()) != ESP_OK) return err;

    /** Décodage vers structure utilisateur */
    nvm.decode_all(raw_nvm.data());
    return ESP_OK;
}

esp_err_t STUSB4500NVM::write_nvm(const NVMData& nvm)
{
    esp_err_t err;
    uint8_t val;

    /** Préparer les données encodées à écrire */
    std::array<uint8_t, 5 * NVM_SECTOR_SIZE> raw_nvm = nvm.to_array();

    /** 2.3.1.1 – NVM Accessibility */
    val = FTP_CUST_PASSWORD;
    if ((err = i2c.write(REG_FTP_KEY, &val, 1)) != ESP_OK) return err;

    /** 2.3.1.2 – NVM Power-up Sequence */
    val = 0x00;
    if ((err = i2c.write(REG_RW_BUFFER, &val, 1)) != ESP_OK) return err; // Clear RW_BUFFER[0]

    val = FTP_CUST_RESET;
    if ((err = i2c.write(REG_FTP_CTRL_0, &val, 1)) != ESP_OK) return err;

    esp_rom_delay_us(10);
    esp_rom_delay_us(1000);

    val = FTP_CUST_RST_N;
    if ((err = i2c.write(REG_FTP_CTRL_0, &val, 1)) != ESP_OK) return err;

    /** 2.3.1.3 – Full Erase */

    val = static_cast<uint8_t>(FtpOpcode::ERASE_SETUP);
    if ((err = i2c.write(REG_FTP_CTRL_1, &val, 1)) != ESP_OK) return err;

    val = FTP_CUST_REQ | FTP_CUST_RST_N;
    if ((err = i2c.write(REG_FTP_CTRL_0, &val, 1)) != ESP_OK) return err;
    esp_rom_delay_us(1000);

    val = static_cast<uint8_t>(FtpOpcode::ERASE_LOAD);
    if ((err = i2c.write(REG_FTP_CTRL_1, &val, 1)) != ESP_OK) return err;

    val = FTP_CUST_REQ | FTP_CUST_RST_N;
    if ((err = i2c.write(REG_FTP_CTRL_0, &val, 1)) != ESP_OK) return err;
    esp_rom_delay_us(5000);

    val = static_cast<uint8_t>(FtpOpcode::ERASE_EXEC);
    if ((err = i2c.write(REG_FTP_CTRL_1, &val, 1)) != ESP_OK) return err;

    val = FTP_CUST_REQ | FTP_CUST_RST_N;
    if ((err = i2c.write(REG_FTP_CTRL_0, &val, 1)) != ESP_OK) return err;
    esp_rom_delay_us(5000);

    /** 2.3.1.4 to 2.3.1.8 – Write each sector */
    for (uint8_t sector = 0; sector < 5; ++sector)
    {
        const uint8_t* buffer = &raw_nvm[sector * NVM_SECTOR_SIZE];

        if ((err = i2c.write(REG_RW_BUFFER, buffer, NVM_SECTOR_SIZE)) != ESP_OK) return err;
        esp_rom_delay_us(1000);

        val = static_cast<uint8_t>(FtpOpcode::LOAD);
        if ((err = i2c.write(REG_FTP_CTRL_1, &val, 1)) != ESP_OK) return err;

        val = FTP_CUST_REQ | FTP_CUST_RST_N;
        if ((err = i2c.write(REG_FTP_CTRL_0, &val, 1)) != ESP_OK) return err;
        esp_rom_delay_us(1000);

        val = static_cast<uint8_t>(FtpOpcode::PROG);
        if ((err = i2c.write(REG_FTP_CTRL_1, &val, 1)) != ESP_OK) return err;

        val = FTP_CUST_REQ | FTP_CUST_RST_N | (FTP_CUST_SECT + sector);
        if ((err = i2c.write(REG_FTP_CTRL_0, &val, 1)) != ESP_OK) return err;
        esp_rom_delay_us(2000);
    }

    // Sortie du mode test
    if ((err = exit_test_mode()) != ESP_OK) return err;

    // === Post-vérification ===
    NVMData readback;
    if ((err = read_nvm(readback)) != ESP_OK) return err;

    std::array<uint8_t, 40> written = raw_nvm;
    std::array<uint8_t, 40> reread = readback.to_array();

    if (!nvm.equals(reread)) {
        ESP_LOGW(TAG, "Échec de vérification post-écriture : contenu NVM différent !");
        nvm.print_diff(reread, ESP_LOGW);
        return ESP_ERR_INVALID_RESPONSE;
    }

    return ESP_OK;
}

esp_err_t STUSB4500NVM::exit_test_mode()
{
    esp_err_t err;
    uint8_t val = FTP_CUST_RESET;

    if ((err = i2c.write(REG_FTP_CTRL_0, &val, 1)) != ESP_OK) return err;

    val = 0x00;
    return i2c.write(REG_FTP_KEY, &val, 1);
}


ConfigNVM ConfigNVM::fromNVM(const NVMData& nvm) {
    ConfigNVM cfg;

    // --- Bank1 ---
    cfg.gpio_cfg = static_cast<GPIOFunction>(nvm.bank1.gpio_cfg);
    cfg.vbus_only_above_5v = nvm.bank1.vbus_dchg_mask;
    cfg.vbus_disch_time_to_0v = nvm.bank1.vbus_disch_time_to_pdo;
    cfg.vbus_disch_time_to_pdo = nvm.bank1.discharge_time_to_0v;

    // --- Bank3 ---
    cfg.pdo_count = nvm.bank3.dpm_snk_pdo_numb;
    cfg.usb_comm_capable = nvm.bank3.usb_comm_capable;
    cfg.unconstrained_power = nvm.bank3.snk_uncons_power;

    cfg.pdo1.voltage_mv = 5000; // Valeur fixe non codée dans la NVM
    cfg.pdo1.current_ma = nvm.bank3.lut_snk_pdo1 * 500;
    cfg.pdo1.enabled = true;

    cfg.pdo2.voltage_mv = nvm.bank4.snk_pdo_flex1_v * 50;
    cfg.pdo2.current_ma = nvm.bank3.lut_snk_pdo2 * 500;
    cfg.pdo2.enabled = (cfg.pdo_count >= 2);

    cfg.pdo3.voltage_mv = nvm.bank4.snk_pdo_flex2_v * 50;
    cfg.pdo3.current_ma = nvm.bank3.lut_snk_pdo3 * 500;
    cfg.pdo3.enabled = (cfg.pdo_count >= 3);

    // --- Bank4 ---
    cfg.use_flexible_voltage = true;
    cfg.use_flexible_current = true;
    cfg.flexible_current_ma = nvm.bank4.snk_pdo_flex_i * 10;
    cfg.pdo2_voltage_mv = cfg.pdo2.voltage_mv;
    cfg.pdo3_voltage_mv = cfg.pdo3.voltage_mv;

    cfg.pdo1_thresholds.lower_percent = nvm.bank3.snk_ll_pdo1;
    cfg.pdo1_thresholds.upper_percent = nvm.bank3.snk_hl_pdo1;
    cfg.pdo2_thresholds.lower_percent = nvm.bank3.snk_ll_pdo2;
    cfg.pdo2_thresholds.upper_percent = nvm.bank3.snk_hl_pdo2;
    cfg.pdo3_thresholds.lower_percent = nvm.bank3.snk_ll_pdo3;
    cfg.pdo3_thresholds.upper_percent = nvm.bank3.snk_hl_pdo3;

    cfg.power_ok_cfg = nvm.bank4.power_ok_cfg;
    cfg.req_src_current = nvm.bank4.req_src_current;

    return cfg;
}

bool equals(const std::array<uint8_t, 40>& other) const {
    return to_array() == other;
}

std::vector<std::tuple<size_t, uint8_t, uint8_t>> diff(const std::array<uint8_t, 40>& other) const {
    std::vector<std::tuple<size_t, uint8_t, uint8_t>> differences;
    auto current = to_array();

    for (size_t i = 0; i < current.size(); ++i) {
        if (current[i] != other[i]) {
            differences.emplace_back(i, other[i], current[i]); // (offset, original, modified)
        }
    }
    return differences;
}

void print_diff(const std::array<uint8_t, 40>& other, esp_log_level_t level) const {
    auto diffs = diff(other);
    for (const auto& [i, before, after] : diffs) {
        esp_log_write(level, TAG,
                      "Offset 0x%02X: 0x%02X -> 0x%02X",
                      static_cast<int>(i),
                      static_cast<int>(before),
                      static_cast<int>(after));
    }

    if (diffs.empty()) {
        esp_log_write(level, TAG, "Aucune différence détectée dans la NVM.");
    }
}

void ConfigNVM::applyTo(NVMData& nvm) const {
    // --- Bank1 ---
    nvm.bank1.gpio_cfg = static_cast<uint8_t>(gpio_cfg);
    nvm.bank1.vbus_dchg_mask = vbus_only_above_5v;
    nvm.bank1.vbus_disch_time_to_pdo = vbus_disch_time_to_0v;
    nvm.bank1.discharge_time_to_0v = vbus_disch_time_to_pdo;

    // --- Bank3 ---
    nvm.bank3.dpm_snk_pdo_numb = pdo_count;
    nvm.bank3.usb_comm_capable = usb_comm_capable;
    nvm.bank3.snk_uncons_power = unconstrained_power;

    nvm.bank3.lut_snk_pdo1 = pdo1.current_ma / 500;
    nvm.bank3.lut_snk_pdo2 = pdo2.current_ma / 500;
    nvm.bank3.lut_snk_pdo3 = pdo3.current_ma / 500;

    nvm.bank3.snk_ll_pdo1 = pdo1_thresholds.lower_percent;
    nvm.bank3.snk_hl_pdo1 = pdo1_thresholds.upper_percent;
    nvm.bank3.snk_ll_pdo2 = pdo2_thresholds.lower_percent;
    nvm.bank3.snk_hl_pdo2 = pdo2_thresholds.upper_percent;
    nvm.bank3.snk_ll_pdo3 = pdo3_thresholds.lower_percent;
    nvm.bank3.snk_hl_pdo3 = pdo3_thresholds.upper_percent;

    // --- Bank4 ---
    nvm.bank4.snk_pdo_flex1_v = pdo2_voltage_mv / 50;
    nvm.bank4.snk_pdo_flex2_v = pdo3_voltage_mv / 50;
    nvm.bank4.snk_pdo_flex_i = flexible_current_ma / 10;
    nvm.bank4.power_ok_cfg = power_ok_cfg;
    nvm.bank4.req_src_current = req_src_current;
}