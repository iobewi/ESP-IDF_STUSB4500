#pragma once

#include <stdint.h>
#include <array>
#include <iostream>
#include <cstring>


namespace stusb4500 {

    // Taille d'un secteur dans la mémoire NVM (8 octets)
    constexpr size_t NVM_SECTOR_SIZE = 8;

    
    // Structure représentant la Bank0 dans la mémoire NVM
    struct Bank0 {
        void decode(const uint8_t*) {}  // Rien à faire
        void encode(uint8_t*) const {}  // On laisse la trame par défaut
    };

    // Structure représentant la Bank1 dans la mémoire NVM
    struct Bank1 {
        uint8_t gpio_cfg;               // Bits 4:5 à l'adresse 0xC8 (Configuration GPIO)
        uint8_t vbus_dchg_mask;         // Bit 5 à l'adresse 0xC9 (Masque de décharge VBUS)
        uint8_t vbus_disch_time_to_pdo; // Bits 0:3 à l'adresse 0xCA (Temps de décharge de VBUS vers PDO)
        uint8_t discharge_time_to_0v;   // Bits 4:7 à l'adresse 0xCA (Temps de décharge vers 0V)

    
        // Fonction pour décoder les données de la Bank1
        void decode(const uint8_t* buffer) {
            // gpio_cfg : bits 4:5 de l'octet à l'adresse 0xC8
            gpio_cfg = (buffer[0] >> 4) & 0x03;  // Décalage de 4 bits pour obtenir les bits 4 et 5
    
            // vbus_dchg_mask : bit 5 de l'octet à l'adresse 0xC9
            vbus_dchg_mask = (buffer[1] >> 5) & 0x01;  // Décalage de 5 bits pour obtenir le bit 5
    
            // vbus_disch_time_to_pdo : bits 0:3 de l'octet à l'adresse 0xCA
            vbus_disch_time_to_pdo = buffer[2] & 0x0F; // Extraction des bits 0 à 3

            // discharge_time_to_0v : bits 4:7 de l'octet à l'adresse 0xCA
            discharge_time_to_0v = (buffer[2] >> 4) & 0x0F;   // Décalage de 4 bits pour obtenir les bits 4 à 7
        }

        // Fonction pour encoder Bank 1
        void encode(uint8_t* buffer) const {
            // gpio_cfg : bits 4-5 (2 bits)
            buffer[0] &= 0xCF; // 11001111 : efface bits 4-5
            buffer[0] |= (gpio_cfg & 0x03) << 4; // masque défensif 0b00000011
        
            // vbus_dchg_mask : bit 5 (1 bit)
            buffer[1] &= 0xDF; // 11011111 : efface bit 5
            buffer[1] |= (vbus_dchg_mask & 0x01) << 5; // masque défensif 0b00000001
        
            // vbus_disch_time_to_pdo : bits 0-3 (4 bits)
            // discharge_time_to_0v   : bits 4-7 (4 bits)
            buffer[2] &= 0x00; // efface tout
            buffer[2] |= (vbus_disch_time_to_pdo & 0x0F);         // 0b00001111
            buffer[2] |= (discharge_time_to_0v   & 0x0F) << 4;    // 0b00001111 << 4
        }
    };

    // Structure représentant la Bank2 dans la mémoire NVM
    struct Bank2 {    
        void decode(const uint8_t*) {}  // Rien à faire
        void encode(uint8_t*) const {}  // On laisse la trame par défaut
    };
    
    // Structure représentant la Bank3 dans la mémoire NVM
    struct Bank3 {
        bool snk_uncons_power;      // SNK_UNCONS_POWER (Bit 3 de 0xDA)
        uint8_t dpm_snk_pdo_numb;   // DPM_SNK_PDO_NUMB (Bits 1-2 de 0xDA)
        bool usb_comm_capable;      // USB_COMM_CAPABLE (Bit 0 de 0xDA)

        uint8_t lut_snk_pdo1;       // LUT_SNK_PDO1 (Bits 4-7 de 0xDA)
        uint8_t snk_hl_pdo1;            // SNK_HL (Bits 4-7 de 0xDB)
        uint8_t snk_ll_pdo1;         // SNK_LL[2:3] (Bits 0-3 de 0xDB)

        uint8_t lut_snk_pdo2;       // LUT_SNK_PDO2[3:0] (Bits 0-3 de 0xDC)
        uint8_t snk_hl_pdo2;            // SNK_HL[13:0] (Bits 4-7 de 0xDD)
        uint8_t snk_ll_pdo2;         // SNK_LL[2:3] (Bits 0-3 de 0xDC)

        uint8_t lut_snk_pdo3;       // LUT_SNK_PDO2[3:0] (Bits 0-3 de 0xDD)
        uint8_t snk_hl_pdo3;         // SNK_HL[3:0] (Bits 0-3 de 0xDE)
        uint8_t snk_ll_pdo3;         // SNK_LL[3:0] (Bits 0-3 de 0xDE)

        uint8_t snk_pdo_fill;         // 0xDF
    
        // Fonction pour décoder les données de la Bank3
        void decode(const uint8_t* buffer) {

            // 0xDA
            usb_comm_capable = buffer[2]  & 0x01;  // Bit 0 de 0xDA
            dpm_snk_pdo_numb = (buffer[2] >> 1) & 0x03;  // Bits 1-2 de 0xDA
            snk_uncons_power = (buffer[2] >> 3) & 0x01;  // Bit 3 de 0xDA
            lut_snk_pdo1 = (buffer[2] >> 4) & 0x0F;  // Bits 4-7 de 0xDA

            // 0xDB
            snk_ll_pdo1 = buffer[3] & 0x0F;  // Bits 0-3 de 0xDB
            snk_hl_pdo1 =(buffer[3] >> 4) & 0x0F;  // Bits 4-7 de 0xDB

            // 0xDC
            lut_snk_pdo2 = buffer[4] & 0x0F;  // Bits 0-3 de 0xDC
            snk_ll_pdo2 = (buffer[4] >> 4) & 0x0F;  // Bits 4-7 de 0xDC

            // 0xDD
            snk_hl_pdo2 = buffer[5] & 0x0F;  // Bits 0-3 de 0xDD
            lut_snk_pdo3 = (buffer[5] >> 4) & 0x0F;  // Bits 4-7 de 0xDD

            // 0xDE
            snk_ll_pdo3 = buffer[6] & 0x0F;  // Bits 0-3 de 0xDE
            snk_hl_pdo3 =(buffer[6] >> 4) & 0x0F;  // Bits 4-7 de 0xDE
            snk_pdo_fill = buffer[7];

        }

        // Fonction pour encoder Bank 3
        void encode(uint8_t* buffer) const {
            // 0xDA
            buffer[2] &= 0x00;
            buffer[2] |= (usb_comm_capable   & 0x01);         // bit 0
            buffer[2] |= (dpm_snk_pdo_numb   & 0x03) << 1;     // bits 1-2
            buffer[2] |= (snk_uncons_power   & 0x01) << 3;     // bit 3
            buffer[2] |= (lut_snk_pdo1       & 0x0F) << 4;     // bits 4-7
        
            // 0xDB
            buffer[3] &= 0x00;
            buffer[3] |= (snk_ll_pdo1        & 0x0F);          // bits 0-3
            buffer[3] |= (snk_hl_pdo1        & 0x0F) << 4;     // bits 4-7
        
            // 0xDC
            buffer[4] &= 0x00;
            buffer[4] |= (lut_snk_pdo2       & 0x0F);          // bits 0-3
            buffer[4] |= (snk_ll_pdo2        & 0x0F) << 4;     // bits 4-7
        
            // 0xDD
            buffer[5] &= 0x00;
            buffer[5] |= (snk_hl_pdo2        & 0x0F);          // bits 0-3
            buffer[5] |= (lut_snk_pdo3       & 0x0F) << 4;     // bits 4-7
        
            // 0xDE
            buffer[6] &= 0x00;
            buffer[6] |= (snk_ll_pdo3        & 0x0F);          // bits 0-3
            buffer[6] |= (snk_hl_pdo3        & 0x0F) << 4;     // bits 4-7
        }
    };
    
    // Structure représentant la Bank4 dans la mémoire NVM
    struct Bank4 {
        uint16_t snk_pdo_flex1_v;   // [1:0] Bits 6-7 de 0xE0 et [9:2] 0xE1
        uint16_t snk_pdo_flex2_v;   // [7:0] 0xE2 et  [9:8] Bits 0-1 de 0xE3
        uint16_t snk_pdo_flex_i;   // [5:0] Bits 2-7 de 0xE3 et [9:6] Bits 0-3 de 0xE4
        uint8_t power_ok_cfg;       // [1:0] Bits 5-6 de 0xE4  
        uint8_t spare;             // 0xE5
        uint8_t req_src_current;    // Bits 4 de 0xE6
        uint8_t alert_status_mask;         // 0xE7
    
        // Fonction pour décoder les données de la Bank4
        void decode(const uint8_t* buffer) {
            snk_pdo_flex1_v = ((buffer[0] >> 6) & 0x03) | (buffer[1] << 2) ; 
            snk_pdo_flex2_v = (buffer[2]  | (buffer[3] &  0x03 )<< 8) ; 
            snk_pdo_flex_i = ((buffer[3] >> 2) & 0x3F) | ((buffer[4] & 0x0F) << 6);
            power_ok_cfg = (buffer[4] >> 5) & 0x03;
            spare = buffer[5];
            req_src_current = (buffer[6] >> 4) & 0x01;
            alert_status_mask = buffer[7];
        }

        // Fonction pour encoder Bank 4
        void encode(uint8_t* buffer) const {
            // 0xE0 : bits 6-7 = [1:0] de snk_pdo_flex1_v
            buffer[0] &= 0x3F; // 0011 1111 : efface bits 6-7
            buffer[0] |= ((snk_pdo_flex1_v & 0x03) << 6);
        
            // 0xE1 : bits 7-0 = [9:2] de snk_pdo_flex1_v
            buffer[1] = (snk_pdo_flex1_v >> 2) & 0xFF;
        
            // 0xE2 : bits 7-0 = [7:0] de snk_pdo_flex2_v
            buffer[2] = snk_pdo_flex2_v & 0xFF;
        
            // 0xE3 :
            buffer[3] &= 0x00; // efface tout l’octet pour simplifier
            buffer[3] |= (snk_pdo_flex2_v >> 8) & 0x03;               // bits 0-1 = [9:8]
            buffer[3] |= ((snk_pdo_flex_i & 0x3F) << 2);              // bits 2-7 = [5:0]
        
            // 0xE4 :
            buffer[4] &= 0x00; // efface tout
            buffer[4] |= ((snk_pdo_flex_i >> 6) & 0x0F);              // bits 0-3 = [9:6]
            buffer[4] |= (power_ok_cfg & 0x03) << 5;                  // bits 5-6

            // 0xE6 :
            buffer[6] &= 0xEF; // efface bit 4
            buffer[6] |= (req_src_current & 0x01) << 4;
        }
    };

    // Structure pour stocker toutes les Bank dans la NVM
    struct NVMData {
        Bank0 bank0;   // Bank 0
        Bank1 bank1;   // Bank 1
        Bank2 bank2;   // Bank 2
        Bank3 bank3;   // Bank 3
        Bank4 bank4;   // Bank 4
    
        // Fonction pour décoder toutes les Bank en une seule opération
        void decode_all(const uint8_t* buffer) {
            bank1.decode(&buffer[8]);
            bank3.decode(&buffer[24]);
            bank4.decode(&buffer[32]);
        }

        // Écriture dans une trame brute (avec préremplissage par défaut)
        void encode_all(uint8_t* buffer) const {
            bank1.encode(&buffer[8]);
            bank3.encode(&buffer[24]);
            bank4.encode(&buffer[32]);
        }

        // Retourne un tableau complet prêt à être écrit
        std::array<uint8_t, 40> to_array() const {
            std::array<uint8_t, 40> result = default_nvm_map;
            encode_all(result.data());
            return result;
        }

        // Trame par défaut basée sur la datasheet (valeurs initiales ST)
        static constexpr std::array<uint8_t, 40> default_nvm_map = {
            0x00, 0x00, 0xB0, 0xAA, 0x00, 0x45, 0x00, 0x00, // Bank 0
            0x10, 0x40, 0x9C, 0x1C, 0xFF, 0x01, 0x3C, 0xDF, // Bank 1
            0x02, 0x40, 0x0F, 0x00, 0x32, 0x00, 0xFC, 0xF1, // Bank 2
            0x00, 0x19, 0x56, 0xAF, 0xF5, 0x35, 0x5F, 0x00, // Bank 3
            0x00, 0x4B, 0x90, 0x21, 0x00, 0x00, 0x00, 0xFB  // Bank 4
        };
    };
}
