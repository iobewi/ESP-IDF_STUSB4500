#pragma once

#include "esp_err.h"
#include <cstdint>

#include "stusb4500-interface.hpp"
#include "nvm/stusb4500-nvm_data.hpp"

namespace stusb4500
{
    class NVM : public INTERFACE
    {
    public:
        explicit NVM(I2CDevices &dev) : INTERFACE(dev) {}

        esp_err_t read(NVMData &nvm);

        esp_err_t write(const NVMData &nvm);

    private:
        esp_err_t write_ctrl0(uint8_t flags);
        esp_err_t write_ctrl1(uint8_t flags);


        // Adresses des registres de contrôle
        static constexpr uint8_t REG_FTP_KEY = 0x95;
        static constexpr uint8_t REG_FTP_CTRL_0 = 0x96;
        static constexpr uint8_t REG_FTP_CTRL_1 = 0x97;

        // Adresse de base des secteurs (FTP_CUST_SECT est dans FTP_CTRL_0[2:0], mais 0x50 est utilisée pour write_register)
        static constexpr uint8_t REG_SECTOR_ADDR = 0x50;

        // Adresses du buffer de lecture/écriture de la NVM (8 octets)
        static constexpr uint8_t REG_RW_BUFFER = 0x53; // Jusqu’à 0x5A

        // Bits de FTP_CTRL_0
        static constexpr uint8_t FTP_CUST_PWR = 1 << 7;   // [7] PWR (non utilisé mais toujours mis à 1 pour FTP_CUST_RUN)
        static constexpr uint8_t FTP_CUST_RST_N = 1 << 6; // [6] Reset Not (1 = pas de reset, 0 = reset)
        static constexpr uint8_t FTP_CUST_REQ = 1 << 4;   // [4] Request (pour valider une opération)

        // Masques pour accéder à FTP_CUST_SECT (bits [2:0])
        static constexpr uint8_t FTP_CUST_SECT_MASK = 0b00000111;

        // Valeurs standards
        static constexpr uint8_t FTP_CUST_RESET = 0x00; // Reset complet
        static constexpr uint8_t FTP_CUST_RUN = FTP_CUST_PWR | FTP_CUST_RST_N;

        // Clé FTP pour activer les accès NVM
        static constexpr uint8_t FTP_CUST_PASSWORD = 0x47;

        // Taille des secteurs NVM
        static constexpr uint8_t NVM_SECTOR_SIZE = 8;
        static constexpr uint8_t NVM_NB_BANKS = 5;
        static constexpr std::size_t NVM_TOTAL_SIZE = NVM_SECTOR_SIZE * NVM_NB_BANKS;

        // Énumération des opérations valides sur FTP_CTRL_1
        enum class FtpOpcode : uint8_t
        {
            READ = 0x00,        // Lecture depuis EEPROM vers buffer
            LOAD = 0x01,        // Chargement du buffer vers registre intermédiaire
            PROG = 0x06,        // Écriture vers EEPROM
            ERASE_SETUP = 0xFA, // Préparation effacement
            ERASE_LOAD = 0x07,  // Chargement secteur pour effacement
            ERASE_EXEC = 0x05   // Lancer effacement
            // Autres valeurs réservées à l’usage interne
        };
    };

} // namespace stusb4500
