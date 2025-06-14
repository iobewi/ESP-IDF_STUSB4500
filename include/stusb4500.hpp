#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "sdkconfig.h"

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_intr_alloc.h"

#include "config/stusb4500-config.hpp"
#include "ctrl/stusb4500-ctrl.hpp"
#include "nvm/stusb4500-nvm.hpp"
#include "pd/stusb4500-pdo.hpp"
#include "pd/stusb4500-rdo.hpp"
#include "pd/stusb4500-rx_datas.hpp"
#include "status/stusb4500-status.hpp"

namespace stusb4500
{
    enum class OutputFormat
    {
        None,
        Log,
        JSON
    };

    class STUSB4500Manager
    {
    public:
        STUSB4500Manager(I2CDevices &i2c);

        // === API PUBLIQUE ===

        void init();

        esp_err_t init_device();

        /// Envoie un soft reset au STUSB4500
        esp_err_t reset();

        esp_err_t apply_nvm_config(ConfigParams &cfg);

        esp_err_t check_nvm_config(ConfigParams &cfg);

        esp_err_t handle_alert();
        
        /// Réécrit le PDO avec la configuration par défaut et force une renégociation
        esp_err_t reconfigure(uint8_t index, Config &cfg);

        /// Lit et retourne l’état courant de la connexion USB-C
        esp_err_t get_status(OutputFormat format = OutputFormat::None);
        esp_err_t get_connection_status(OutputFormat format = OutputFormat::None);

        esp_err_t get_active_pdo(OutputFormat format = OutputFormat::None);

    private:
        I2CDevices &i2c_;
        Config cfg_;
        gpio_num_t alert_gpio_;
        STATUS status_;
        CTRL ctrl_;

        TaskHandle_t task_handle_ = nullptr;

        inline static const char *TAG = "STUSB4500_MANAGER";
        bool ready_ = false;
        esp_err_t is_ready();

        static void task_wrapper(void *arg);
        static void IRAM_ATTR gpio_isr_handler(void *arg);
        void setup_interrupt(gpio_num_t gpio);
        void task_main();
    };

} // namespace stusb4500
