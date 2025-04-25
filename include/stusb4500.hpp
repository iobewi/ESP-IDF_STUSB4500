#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_intr_alloc.h"

#include "helpers.hpp"
#include "data-types.hpp"

namespace stusb4500 {

class STUSB4500Manager {
public:

    STUSB4500Manager(I2CDevices& i2c, SinkPDOConfig default_pdo, gpio_num_t alert_gpio)
    : i2c_(i2c),
      default_pdo_(default_pdo),
      alert_gpio_(alert_gpio),
      iface_(i2c), 
      helper_(iface_) {}

    // === API PUBLIQUE ===

    void init() {
        xTaskCreatePinnedToCore(task_wrapper, "STUSB_Task", 4096, this, 5, &task_handle_, 1);
    }

    /// Envoie un soft reset au STUSB4500
    bool reset() {
        return helper_.send_soft_reset();
    }

    /// Réécrit le PDO #3 avec la configuration par défaut et force une renégociation
    void reconfigure() {
        helper_.write_sink_pdo(3, default_pdo_);
        helper_.update_valid_pdo_number(1);
        helper_.send_soft_reset();
    }

    /// Lit et retourne l’état courant de la connexion USB-C
    USBConnectionStatus get_status() {
        return helper_.read_connection_status();
    }

    /// Force une renégociation USB PD
    void renegotiate() {
        helper_.send_soft_reset();
    }
  
    
private:
    I2CDevices& i2c_;
    SinkPDOConfig default_pdo_;
    gpio_num_t alert_gpio_;

    INTERFACE iface_;
    HELPERS helper_;
    TaskHandle_t task_handle_ = nullptr;

    PowerContract active_contract_;

    static void task_wrapper(void* arg) {
        static_cast<STUSB4500Manager*>(arg)->task_main();
    }

    static void IRAM_ATTR gpio_isr_handler(void* arg) {
        auto* self = static_cast<STUSB4500Manager*>(arg);
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        vTaskNotifyGiveFromISR(self->task_handle_, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }

    void setup_interrupt(gpio_num_t gpio) {
        static bool isr_installed = false;
        gpio_config_t io_conf = {};
        io_conf.intr_type = GPIO_INTR_NEGEDGE;
        io_conf.mode = GPIO_MODE_INPUT;
        io_conf.pin_bit_mask = (1ULL << gpio);
        io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
        gpio_config(&io_conf);


        if (!isr_installed) {
            esp_err_t err = gpio_install_isr_service(ESP_INTR_FLAG_LEVEL3);
            if (err == ESP_OK || err == ESP_ERR_INVALID_STATE) {
                isr_installed = true;
            } else {
                ESP_LOGE("STUSB4500", "Failed to install ISR service: %s", esp_err_to_name(err));
            }
        }

        gpio_isr_handler_add(gpio, gpio_isr_handler, this);
    }

    void task_main() {
        uint32_t renegotiation_delay_ms_ = 1000;

        setup_interrupt(alert_gpio_);

        helper_.init_device();
        auto status = helper_.read_connection_status();
        ESP_LOGI("STUSB4500", "Connected: %d, Device: %d", status.attached, static_cast<int>(status.device));

        helper_.write_sink_pdo(2, default_pdo_);
        helper_.update_valid_pdo_number(2);
        helper_.send_soft_reset();
        while (true) {
            if (gpio_get_level(alert_gpio_) == 0 || ulTaskNotifyTake(pdTRUE, portMAX_DELAY)) {
                uint8_t alerts = helper_.read_alert_status();
        
                if (alerts & static_cast<uint8_t>(AlertBit::PORT_STATUS_AL)) {
                    ESP_LOGI("STUSB4500", "[ALERT] PORT_STATUS_AL → Reconnexion ou déconnexion détectée");
                    helper_.log_connection_status();
                }
        
                if (alerts & static_cast<uint8_t>(AlertBit::TYPEC_MONITORING_STATUS_AL)) {
                    ESP_LOGI("STUSB4500", "[ALERT] TYPEC_MONITORING_STATUS_AL → Changement de ligne CC");
                    // Action possible : log, vérification d’état, etc.
                }
        
                if (alerts & static_cast<uint8_t>(AlertBit::CC_HW_FAULT_STATUS_AL)) {
                    ESP_LOGW("STUSB4500", "[ALERT] CC_HW_FAULT_STATUS_AL → Défaut matériel CC détecté !");
                    // Action : log ou reconfiguration
                }
        
                if (alerts & static_cast<uint8_t>(AlertBit::PRT_STATUS_AL)) {
                    auto prt = helper_.read_prt_status();
                    RXMessage msg = helper_.read_rx_message();
                    //prt.log();
                    if (prt.msg_received() && msg.is_data_message() ) {
                        auto contract = helper_.power_negotiation(msg);
                        ESP_LOGI("STUSB4500", "[ALERT] PRT_STATUS_AL → Message PD reçu");
                        // Lire PRT_STATUS et vérifier MSG_RECEIVED
                        if (!contract.valid) {
                            ESP_LOGW("STUSB4500", "Renegotiation triggered...");
                            helper_.send_soft_reset();
                            active_contract_ = {};  // Réinitialisation
                        } else {
                            active_contract_ = contract;
                            active_contract_.log();  // Affiche les infos
                        }
                    }
                }
            }
        }
    }
};

} // namespace stusb4500
