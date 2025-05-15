#include "nvm/stusb4500-nvm.hpp"

#define RETURN_IF_ERROR(x)         \
    do {                           \
        esp_err_t __err_rc = (x);  \
        if (__err_rc != ESP_OK) {  \
            return __err_rc;       \
        }                          \
    } while (0)

namespace stusb4500
{
    static const char *TAG = "STUSB4500-NVM";


    esp_err_t NVM::write_ctrl0(uint8_t flags)
    {
        esp_err_t err = write_register(REG_FTP_CTRL_0, &flags, 1);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Échec write REG_FTP_CTRL_0 avec flags: 0x%02X", flags);
        }
        return err;
    }


    esp_err_t NVM::write_ctrl1(uint8_t flags)
    {
        esp_err_t err = write_register(REG_FTP_CTRL_1, &flags, 1);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Échec write REG_FTP_CTRL_1 avec flags: 0x%02X", flags);
        }
        return err;
    }


    esp_err_t NVM::read(NVMData &nvm)
    {
        std::array<uint8_t, NVM_TOTAL_SIZE> full_buffer;

        /** 2.2.1.1 – NVM Accessibility */
        uint8_t password = FTP_CUST_PASSWORD;
        RETURN_IF_ERROR(write_register(REG_FTP_KEY, &password, 1));

        /** 2.2.1.2 – NVM Power-up Sequence */
        RETURN_IF_ERROR(write_ctrl0(FTP_CUST_RESET));    
        esp_rom_delay_us(1000);
        RETURN_IF_ERROR(write_ctrl0(FTP_CUST_RST_N)); 

        // 3. Définir l'opcode READ (une seule fois)
        for (uint8_t sector = 0; sector < NVM_NB_BANKS; ++sector)
        {
            RETURN_IF_ERROR(write_ctrl1(static_cast<uint8_t>(FtpOpcode::READ)));
            RETURN_IF_ERROR(write_ctrl0(FTP_CUST_RST_N | FTP_CUST_REQ | (sector & 0x07))); 
            esp_rom_delay_us(1000);
            // 7. Lire les 8 octets du buffer
            RETURN_IF_ERROR(read_register(REG_RW_BUFFER, &full_buffer[sector * NVM_SECTOR_SIZE], NVM_SECTOR_SIZE));

        }

        uint8_t ctrl_reset[] = {FTP_CUST_PWR, 0x00}; // 0x40, 0x00
        RETURN_IF_ERROR(write_register(REG_FTP_CTRL_0, ctrl_reset, 2)); // écrit dans 0x96 et 0x97

        uint8_t lock = 0x00;
        RETURN_IF_ERROR(write_register(REG_FTP_KEY, &lock, 1));

        /** Décodage vers structure utilisateur */
        nvm.decode(full_buffer.data());
        return ESP_OK;
    }

    esp_err_t NVM::write(const NVMData &nvm)
    {
        esp_err_t err;
        std::array<uint8_t, NVM_TOTAL_SIZE> raw_nvm = nvm.to_array();
        // 1. NVM Accessibility
         uint8_t password = FTP_CUST_PASSWORD;
         RETURN_IF_ERROR(write_register(REG_FTP_KEY, &password, 1));
 

        // 2. NVM Power-up Sequence
        uint8_t data = 0x00;
        RETURN_IF_ERROR(write_register(REG_RW_BUFFER, &data, 1));

        RETURN_IF_ERROR(write_ctrl0(FTP_CUST_RESET));    
        esp_rom_delay_us(1000);

        RETURN_IF_ERROR(write_ctrl0(FTP_CUST_RST_N)); 

        // 3. Full Erase
        RETURN_IF_ERROR(write_ctrl1(static_cast<uint8_t>(FtpOpcode::ERASE_SETUP)));

        RETURN_IF_ERROR(write_ctrl0(FTP_CUST_REQ | FTP_CUST_RST_N));    
        esp_rom_delay_us(1000);
    
        RETURN_IF_ERROR(write_ctrl1(static_cast<uint8_t>(FtpOpcode::ERASE_LOAD)));

        RETURN_IF_ERROR(write_ctrl0(FTP_CUST_REQ | FTP_CUST_RST_N));    

        esp_rom_delay_us(5000);

        RETURN_IF_ERROR(write_ctrl1(static_cast<uint8_t>(FtpOpcode::ERASE_EXEC)));

        RETURN_IF_ERROR(write_ctrl0(FTP_CUST_REQ | FTP_CUST_RST_N));   

        esp_rom_delay_us(5000);

        // 4. Write each sector
        for (uint8_t sector = 0; sector < 5; ++sector)
        {
            const uint8_t *buffer = &raw_nvm[sector * NVM_SECTOR_SIZE];

            ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, NVM_SECTOR_SIZE, ESP_LOG_INFO);
            RETURN_IF_ERROR(write_register(REG_RW_BUFFER, buffer, NVM_SECTOR_SIZE));
            esp_rom_delay_us(1000);

            RETURN_IF_ERROR(write_ctrl1(static_cast<uint8_t>(FtpOpcode::LOAD)));
            RETURN_IF_ERROR(write_ctrl0(FTP_CUST_REQ | FTP_CUST_RST_N));  
            esp_rom_delay_us(1000);

            RETURN_IF_ERROR(write_ctrl1(static_cast<uint8_t>(FtpOpcode::PROG)));
            RETURN_IF_ERROR(write_ctrl0(FTP_CUST_RST_N | FTP_CUST_REQ | (sector & 0x07)));  
            esp_rom_delay_us(2000);
        }

        // Sortie du mode test
        uint8_t ctrl_reset[] = {FTP_CUST_PWR, 0x00}; // 0x40, 0x00
        RETURN_IF_ERROR(write_register(REG_FTP_CTRL_0, ctrl_reset, 2)); // écrit dans 0x96 et 0x97

        uint8_t lock = FTP_CUST_PASSWORD;
        RETURN_IF_ERROR(write_register(REG_FTP_KEY, &lock, 1));


        // Post-vérification
        ConfigParams cfg_data;
        NVMData readback(cfg_data);
        err = read(readback);
        readback.log();
        nvm.log();
        if (err != ESP_OK)
            return err;

        std::array<uint8_t, 40> reread = readback.to_array();

        if (!nvm.equals(reread))
        {
            ESP_LOGW(TAG, "Échec de vérification post-écriture : contenu NVM différent !");
            nvm.print_diff(reread);
            return ESP_ERR_INVALID_RESPONSE;
        }

        return ESP_OK;
    }

} // namespace stusb4500
