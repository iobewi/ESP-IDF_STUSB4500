#pragma once

#include <stdint.h>
#include <cstddef>

namespace stusb4500 {

    // FTP_CTRL_0 (0x96)
    constexpr uint8_t REG_FTP_CTRL_0 = 0x96;

    constexpr uint8_t FTP_CUST_PWR     = 1 << 7; // [7] PWR
    constexpr uint8_t FTP_CUST_RST_N   = 1 << 6; // [6] Reset Not
    constexpr uint8_t FTP_CUST_REQ     = 1 << 4; // [4] Request


    constexpr uint8_t FTP_CUST_RESET   = 0x00;
    constexpr uint8_t FTP_CUST_RUN     = FTP_CUST_PWR | FTP_CUST_RST_N;

    // FTP_CTRL_1 (0x97) – opcode
    constexpr uint8_t REG_FTP_CTRL_1 = 0x97;

    enum class FtpOpcode : uint8_t
    {
        // Lecture de la NVM
        READ         = 0x00, // Lecture de secteur (NVM Read)

        // Chargement de données dans le tampon interne (avant programmation)
        LOAD         = 0x01, // Shift In Data on Program Load Register

        // Programmation EEPROM à partir du tampon
        PROG         = 0x06, // Program word into EEPROM

        // Effacement complet de la NVM (Full erase)
        ERASE_SETUP  = 0xFA, // Shift In Data on Sector Erase Register (all sectors)
        ERASE_LOAD   = 0x07, // Soft Program Array
        ERASE_EXEC   = 0x05, // Erase memory array
    };

    // FTP_KEY (0x95)
    constexpr uint8_t REG_FTP_KEY      = 0x95;
    constexpr uint8_t FTP_CUST_PASSWORD = 0x47;

    // Registres de transfert (RW_BUFFER)
    constexpr uint8_t REG_RW_BUFFER = 0x53;
    constexpr uint8_t NVM_SECTOR_SIZE = 8;

    // Base pour les secteurs NVM
    constexpr uint8_t FTP_CUST_SECT = 0x50;

}
