#pragma once

#include "I2CDevice.hpp"
#include "esp_err.h"
#include "stusb4500_regs.hpp"
#include "stusb4500_nvmdata.hpp"
#include "stusb4500_confignvm.hpp"
#include <array>

/**
 * @class STUSB4500NVM
 * @brief Gère la lecture/écriture des secteurs NVM du STUSB4500.
 */
class STUSB4500NVM {
public:
    explicit STUSB4500NVM(I2CDevice& device);

    /**
     * @brief Lit l'intégralité des 5 secteurs NVM (5 × 8 octets).
     * 
     * Cette fonction suit rigoureusement la section 2.2.1 de la documentation ST.
     * Elle est optimisée pour ne pas répéter la séquence d'activation NVM.
     * 
     * @param[out] nvm_data Buffer de 40 octets contenant les 5 secteurs lus.
     * @return esp_err_t Code d'erreur ESP-IDF
     */
    esp_err_t read_nvm(NVMData& nvm);

    /**
     * @brief Écrit l'intégralité des 5 secteurs de la NVM du STUSB4500.
     *
     * Cette méthode suit rigoureusement la séquence décrite dans la section 2.3.1
     * de la documentation officielle ST ("NVM WRITE procedure").
     *
     * Elle exécute les étapes suivantes :
     *  - Déverrouillage de l'accès client (`FTP_KEY`)
     *  - Séquence d'initialisation et mise sous tension du moteur NVM
     *  - Effacement complet des 5 secteurs NVM (`ERASE_SETUP`, `ERASE_LOAD`, `ERASE_EXEC`)
     *  - Programmation séquentielle des 5 secteurs (`LOAD` → `PROG` pour chaque secteur)
     *  - Fermeture sécurisée du mode test (reset `FTP_CTRL_0` et verrouillage `FTP_KEY`)
     *
     * ⚠️ Cette opération est lente (environ 30 ms) et doit être utilisée avec précaution :
     *    une programmation excessive peut réduire la durée de vie de la mémoire.
     *
     * @param[in] nvm_data Tableau de 40 octets à programmer dans la NVM (5 secteurs × 8 octets).
     *                     L'ordre doit respecter les secteurs 0 à 4 consécutivement.
     *
     * @return esp_err_t Code ESP_OK si succès, ou une erreur ESP-IDF sinon.
     */
    esp_err_t write_nvm(const NVMData& nvm);


    /**
     * @brief Compare le contenu d’un secteur avec des données attendues
     * @param sector Numéro de secteur (0 à 4)
     * @param expected Données de référence
     * @return true si différent, false sinon
     */
    bool compare_sector(uint8_t sector, const std::array<uint8_t, stusb4500::NVM_SECTOR_SIZE>& expected);

    /**
     * @brief Compare tous les secteurs avec une configuration par défaut
     * @return true si au moins un secteur est différent
     */
    bool is_config_different() const;

    /**
     * @brief Écrit tous les secteurs à partir d’une configuration par défaut compilée
     * @return esp_err_t
     */
    esp_err_t write_default_config();

    /**
     * @brief Écrit les secteurs NVM uniquement si la configuration actuelle est différente
     * @return esp_err_t
     */
    esp_err_t apply_default_if_needed();

    esp_err_t decode_nvm(const stusb4500::NVMData& nvmData, stusb4500::ConfigNVM& config);

private:
    I2CDevice& i2c;
    
    /**
     * @brief Quitte proprement le mode FTP/NVM du STUSB4500.
     */
    esp_err_t exit_test_mode();

    /**
     * @brief Accès à la configuration compilée en dur (5 secteurs x 8 octets)
     */
    const std::array<std::array<uint8_t, stusb4500::NVM_SECTOR_SIZE>, 5> default_sectors;

    stusb4500::NVMData nvmData; 
};
