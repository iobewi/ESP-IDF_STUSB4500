#include "esp_log.h"
#include "freertos/task.h"
#include "esp_rom_sys.h"

#include "nvm/helpers.hpp"
#include "nvm/regs.hpp"

using namespace stusb4500;

static const char *TAG = "NVM";

NVM::NVM(INTERFACE& stusb4500_device)
: device(stusb4500_device) 
{}

esp_err_t NVM::read_nvm(NVMData& nvm)
{
    esp_err_t err;
    uint8_t buffer[NVM_SECTOR_SIZE];


    /** 2.2.1.1 – NVM Accessibility */
    uint8_t password = FTP_CUST_PASSWORD;
    err = device.write_register(REG_FTP_KEY, &password, 1);
    if (err != ESP_OK) return err;

    /** 2.2.1.2 – NVM Power-up Sequence */
    uint8_t ctrl_reset = FTP_CUST_RESET;
    err = device.write_register(REG_FTP_CTRL_0, &ctrl_reset, 1);
    if (err != ESP_OK) return err;
    esp_rom_delay_us(10); // > 2µs comme recommandé
    uint8_t ctrl0 = FTP_CUST_RUN;
    err = device.write_register(REG_FTP_CTRL_0, &ctrl0, 1);
    if (err != ESP_OK) return err;

    // 3. Définir l'opcode READ (une seule fois)
    uint8_t opcode = static_cast<uint8_t>(FtpOpcode::READ);
    err = device.write_register(REG_FTP_CTRL_1, &opcode, 1);
    if (err != ESP_OK) return err;

    for (uint8_t sector = 0; sector < 5; ++sector) {
        // 4. Sélectionner le secteur à lire
        uint8_t addr = FTP_CUST_SECT;
        err = device.write_register(addr, &sector, 1);
        if (err != ESP_OK) return err;

        // 5. Envoyer la commande FTP_CUST_REQ
        uint8_t request = FTP_CUST_RUN | FTP_CUST_REQ;
        err = device.write_register(REG_FTP_CTRL_0, &request, 1);
        if (err != ESP_OK) return err;

        // 6. Délai requis après REQ (1 ms)
        vTaskDelay(pdMS_TO_TICKS(1));

        // 7. Lire les 8 octets du buffer
        err = device.read_register(REG_RW_BUFFER, buffer, NVM_SECTOR_SIZE);
        if (err != ESP_OK) return err;

        // 8. Copier les octets dans le buffer NVMData
        std::memcpy(reinterpret_cast<uint8_t*>(&nvm) + sector * NVM_SECTOR_SIZE, buffer, NVM_SECTOR_SIZE);
    }

    /** Sortie du mode test */
    if ((err = exit_test_mode()) != ESP_OK) return err;

    /** Décodage vers structure utilisateur */
    nvm.decode_all(buffer);
    return ESP_OK;
}

esp_err_t NVM::write_nvm(const NVMData& nvm) {
    esp_err_t err;
    std::array<uint8_t, 5 * NVM_SECTOR_SIZE> raw_nvm = nvm.to_array();

    // 1. NVM Accessibility
    uint8_t val = FTP_CUST_PASSWORD;
    err = device.write_register(REG_FTP_KEY, &val, 1);
    if (err != ESP_OK) return err;

    // 2. NVM Power-up Sequence
    val = 0x00;
    err = device.write_register(REG_RW_BUFFER, &val, 1);
    if (err != ESP_OK) return err;

    val = FTP_CUST_RESET;
    err = device.write_register(REG_FTP_CTRL_0, &val, 1);
    if (err != ESP_OK) return err;

    esp_rom_delay_us(10);
    esp_rom_delay_us(1000);

    val = FTP_CUST_RST_N;
    err = device.write_register(REG_FTP_CTRL_0, &val, 1);
    if (err != ESP_OK) return err;

    // 3. Full Erase
    val = static_cast<uint8_t>(FtpOpcode::ERASE_SETUP);
    err = device.write_register(REG_FTP_CTRL_1, &val, 1);
    if (err != ESP_OK) return err;

    val = FTP_CUST_REQ | FTP_CUST_RST_N;
    err = device.write_register(REG_FTP_CTRL_0, &val, 1);
    if (err != ESP_OK) return err;
    esp_rom_delay_us(1000);

    val = static_cast<uint8_t>(FtpOpcode::ERASE_LOAD);
    err = device.write_register(REG_FTP_CTRL_1, &val, 1);
    if (err != ESP_OK) return err;

    val = FTP_CUST_REQ | FTP_CUST_RST_N;
    err = device.write_register(REG_FTP_CTRL_0, &val, 1);
    if (err != ESP_OK) return err;
    esp_rom_delay_us(5000);

    val = static_cast<uint8_t>(FtpOpcode::ERASE_EXEC);
    err = device.write_register(REG_FTP_CTRL_1, &val, 1);
    if (err != ESP_OK) return err;

    val = FTP_CUST_REQ | FTP_CUST_RST_N;
    err = device.write_register(REG_FTP_CTRL_0, &val, 1);
    if (err != ESP_OK) return err;
    esp_rom_delay_us(5000);

    // 4. Write each sector
    for (uint8_t sector = 0; sector < 5; ++sector) {
        const uint8_t* buffer = &raw_nvm[sector * NVM_SECTOR_SIZE];

        err = device.write_register(REG_RW_BUFFER, buffer, NVM_SECTOR_SIZE);
        if (err != ESP_OK) return err;
        esp_rom_delay_us(1000);

        val = static_cast<uint8_t>(FtpOpcode::LOAD);
        err = device.write_register(REG_FTP_CTRL_1, &val, 1);
        if (err != ESP_OK) return err;

        val = FTP_CUST_REQ | FTP_CUST_RST_N;
        err = device.write_register(REG_FTP_CTRL_0, &val, 1);
        if (err != ESP_OK) return err;
        esp_rom_delay_us(1000);

        val = static_cast<uint8_t>(FtpOpcode::PROG);
        err = device.write_register(REG_FTP_CTRL_1, &val, 1);
        if (err != ESP_OK) return err;

        val = FTP_CUST_REQ | FTP_CUST_RST_N | (FTP_CUST_SECT + sector);
        err = device.write_register(REG_FTP_CTRL_0, &val, 1);
        if (err != ESP_OK) return err;
        esp_rom_delay_us(2000);
    }

    // Sortie du mode test
    err = exit_test_mode();
    if (err != ESP_OK) return err;

    // Post-vérification
    NVMData readback;
    err = read_nvm(readback);
    if (err != ESP_OK) return err;

    std::array<uint8_t, 40> written = raw_nvm;
    std::array<uint8_t, 40> reread = readback.to_array();

    if (!nvm.equals(reread)) {
        ESP_LOGW(TAG, "Échec de vérification post-écriture : contenu NVM différent !");
        nvm.print_diff(reread);
        return ESP_ERR_INVALID_RESPONSE;
    }

    return ESP_OK;
}

esp_err_t NVM::exit_test_mode()
{
    uint8_t reset = FTP_CUST_RESET;
    esp_err_t err = device.write_register(REG_FTP_CTRL_0, &reset, 1);
    if (err != ESP_OK) return err;

    uint8_t lock = 0x00;
    return device.write_register(REG_FTP_KEY, &lock, 1);
}