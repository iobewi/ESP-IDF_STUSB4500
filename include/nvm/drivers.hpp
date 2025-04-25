#pragma once

#include "../drivers.hpp" 
#include "helpers.hpp"
#include "esp_err.h"
#include <array>

namespace stusb4500 {
    /**
     * @class STUSB4500NVM
     * @brief Gère la lecture/écriture des secteurs NVM du STUSB4500.
     */
    class NVM {
    public:
        explicit NVM(INTERFACE& stusb_device);

        esp_err_t read_nvm(NVMData& nvm);

        esp_err_t write_nvm(const NVMData& nvm);

    private:
        esp_err_t exit_test_mode();
        INTERFACE device;
        NVMData nvmData; 
    };
};