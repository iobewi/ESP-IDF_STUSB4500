#include "config/stusb4500-config_macro.hpp"
#include "config/stusb4500-config_types.hpp"
#include "stusb4500.hpp"
#include "sdkconfig.h"

#define RETURN_IF_ERROR(x)         \
    do {                           \
        esp_err_t __err_rc = (x);  \
        if (__err_rc != ESP_OK) {  \
            return __err_rc;       \
        }                          \
    } while (0)

#define HANDLE_OUTPUT(format, obj)                            \
    do {                                                      \
        switch (format)                                       \
        {                                                     \
            case OutputFormat::Log:                           \
                obj.log();                                    \
                break;                                        \
            case OutputFormat::JSON:                          \
                printf("%s\n", obj.to_json().c_str());        \
                break;                                        \
            case OutputFormat::None:                          \
            default:                                          \
                break;                                        \
        }                                                     \
    } while (0)


namespace stusb4500
{

    inline esp_err_t return_if_not_ready(bool ready, const char* tag)
    {
        if (!ready)
        {
            ESP_LOGE(tag, "Tentative d’utilisation sans initialisation préalable");
            return ESP_ERR_INVALID_STATE;
        }
        return ESP_OK;
    }

    STUSB4500Manager::STUSB4500Manager(I2CDevices &i2c)
        : i2c_(i2c),
          cfg_(i2c_),
          alert_gpio_(gpio_num_t(CONFIG_STUSB4500_INT_ALERT)),
          status_(i2c_),
          ctrl_(i2c_)
          {}

    // === API PUBLIQUE ===

    void STUSB4500Manager::init()
    {
        xTaskCreatePinnedToCore(task_wrapper, "STUSB_Task", 4096, this, 5, &task_handle_, 0);
    }

    esp_err_t STUSB4500Manager::init_device()
    {
        RETURN_IF_ERROR(is_ready());
        ConfigParams from_kconfig = load_config_from_kconfig();
        cfg_.datas() = from_kconfig ; 
        cfg_.datas().log();
        RETURN_IF_ERROR(get_status());
        RETURN_IF_ERROR(apply_nvm_config(from_kconfig));
        return ESP_OK;
    }

    esp_err_t STUSB4500Manager::is_ready()
    {
        const int max_attempts = 10;
        const int delay_ms = 50;
        esp_err_t err = ESP_OK;
        for (int attempt = 0; attempt < max_attempts; ++attempt)
        {
            err = ctrl_.ready();
                if (err == ESP_OK)
                {
                    ESP_LOGI(TAG, "STUSB4500 détecté sur le bus (tentative %d)", attempt + 1);
                    break;
                }
            ESP_LOGW(TAG, "Tentative %d/%d : STUSB4500 non prêt (err=0x%x)", attempt + 1, max_attempts, err);
            vTaskDelay(pdMS_TO_TICKS(delay_ms));
        }
        
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "STUSB4500 non détecté après %d tentatives", max_attempts);
            ready_ = false;
            return ESP_ERR_TIMEOUT; // ou err si tu veux refléter la dernière erreur de ready()
        }
        ready_ = true;
        return ESP_OK;
    }

    esp_err_t STUSB4500Manager::apply_nvm_config(ConfigParams &cfg)
    {   
        RETURN_IF_ERROR(return_if_not_ready(ready_, TAG));
        esp_err_t err = check_nvm_config(cfg);
        if ( err == ESP_ERR_INVALID_STATE)
        {
            NVMData new_nvm(cfg);
            NVM iface_nvm(i2c_);
            RETURN_IF_ERROR(iface_nvm.write(new_nvm));
            vTaskDelay(pdMS_TO_TICKS(1000));
            reset(); 
            vTaskDelay(pdMS_TO_TICKS(1000));
            return ESP_OK;
        } else return err;

        return ESP_OK;    
    }

    esp_err_t STUSB4500Manager::check_nvm_config(ConfigParams &cfg)
    {
        RETURN_IF_ERROR(return_if_not_ready(ready_, TAG));
        NVMData new_nvm(cfg);
        ConfigParams active_cfg;
        NVMData active_nvm(active_cfg);
        NVM iface_nvm(i2c_);
        RETURN_IF_ERROR(iface_nvm.read(active_nvm));
        if (! active_nvm.equals(new_nvm.to_array()))
        {
            active_nvm.print_diff(new_nvm.to_array());
            ESP_LOGI(TAG, "Old configuration :");
            active_cfg.log();
            ESP_LOGI(TAG, "New configuration :");
            cfg.log();
            return ESP_ERR_INVALID_STATE;
        }
        return ESP_OK;
    }

    esp_err_t STUSB4500Manager::handle_alert()
    {
        RETURN_IF_ERROR(return_if_not_ready(ready_, TAG));
        RETURN_IF_ERROR(status_.read_alert_status());

        if (status_.alert_status_1.get_values().port_status_al )
        {
            RETURN_IF_ERROR(status_.read_port_status_0());
            status_.port_status_0.log();
        }

        if (status_.alert_status_1.get_values().typec_monitoring_status_al)
        {
            RETURN_IF_ERROR(status_.read_typec_monitoring_status_0());
            status_.typec_monitoring_status_0.log();
        }

        if (status_.alert_status_1.get_values().cc_hw_fault_status_al)
        {
            RETURN_IF_ERROR(status_.read_cc_hw_fault_status_0());
            status_.cc_hw_fault_0.log();
        }

        if (status_.alert_status_1.get_values().prt_status_al)
        {
            RETURN_IF_ERROR(status_.read_prt_status());
            status_.prt_status.log();

            if (status_.prt_status.get_values().prl_hw_rst_received)
            {
                ESP_LOGW(TAG, "PD Hardware Reset detected. Clearing local PD state.");
            }

            if (status_.prt_status.get_values().prt_ibist_received)
            {
                ESP_LOGE(TAG, "PD BIST (Built-In Self Test) mode received! Unexpected behavior!");
            }

            if (status_.prt_status.get_values().prl_msg_received)
            {   
                get_active_pdo(OutputFormat::Log);
            }
        }
        return ESP_OK;
    }

    /// Envoie un soft reset au STUSB4500
    esp_err_t STUSB4500Manager::reset()
    {
        RETURN_IF_ERROR(return_if_not_ready(ready_, TAG));
        return ctrl_.send_soft_reset();
    }

    /// Réécrit le PDO avec la configuration par défaut et force une renégociation
    esp_err_t STUSB4500Manager::reconfigure(uint8_t index, Config &cfg)
    {
        RETURN_IF_ERROR(return_if_not_ready(ready_, TAG));
        PDO active_pdo(i2c_,index,cfg.datas().power_.pdos[index]);
        RETURN_IF_ERROR(active_pdo.write());
        RETURN_IF_ERROR(ctrl_.update_pdo_number(index));
        return ESP_OK;
    }

    /// Lit et retourne l’état courant de la connexion USB-C
    esp_err_t STUSB4500Manager::get_status(OutputFormat format)
    {
        RETURN_IF_ERROR(return_if_not_ready(ready_, TAG));
        RETURN_IF_ERROR(status_.get_status());
        HANDLE_OUTPUT(format, status_);
        return ESP_OK;
    }

    esp_err_t STUSB4500Manager::get_connection_status(OutputFormat format)
    {
        RETURN_IF_ERROR(return_if_not_ready(ready_, TAG));
        RETURN_IF_ERROR(status_.read_port_status_1());
        HANDLE_OUTPUT(format, status_.port_status_1);
        RETURN_IF_ERROR(status_.read_cc_status());
        HANDLE_OUTPUT(format, status_.cc_status);
        return ESP_OK;
    }

    esp_err_t STUSB4500Manager::get_active_pdo(OutputFormat format)
    {
        RETURN_IF_ERROR(return_if_not_ready(ready_, TAG));
        RXDatas rxdatas(i2c_);
        RETURN_IF_ERROR(rxdatas.read());
        RETURN_IF_ERROR(status_.read_policy_engine_state());
        HANDLE_OUTPUT(format, status_.policy_engine_state);
        if (status_.policy_engine_state.get_raw() == 0x18)
        {
            RDO rdo(i2c_);
            RETURN_IF_ERROR(rdo.read());
            uint8_t index = rdo.obj_position();
            if (index >= 1 && index <= cfg_.datas().power_.pdo_number)
            {
                HANDLE_OUTPUT(format, cfg_.datas().power_.pdos[index - 1]);
            }
            else
            {
                ESP_LOGW(TAG, "Index PDO invalide : %u", index);
            }
        }
        return ESP_OK;
    }

    void STUSB4500Manager::task_wrapper(void *arg)
    {
        static_cast<STUSB4500Manager *>(arg)->task_main();
    }

    void IRAM_ATTR STUSB4500Manager::gpio_isr_handler(void *arg)
    {
        auto *self = static_cast<STUSB4500Manager *>(arg);
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        vTaskNotifyGiveFromISR(self->task_handle_, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }

    void STUSB4500Manager::setup_interrupt(gpio_num_t gpio)
    {
        static bool isr_installed = false;
        gpio_config_t io_conf = {};
        io_conf.intr_type = GPIO_INTR_NEGEDGE;
        io_conf.mode = GPIO_MODE_INPUT;
        io_conf.pin_bit_mask = (1ULL << gpio);
        io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
        gpio_config(&io_conf);

        if (!isr_installed)
        {
            esp_err_t err = gpio_install_isr_service(ESP_INTR_FLAG_LEVEL3);
            if (err == ESP_OK || err == ESP_ERR_INVALID_STATE)
            {
                isr_installed = true;
            }
            else
            {
                ESP_LOGE(TAG, "Failed to install ISR service: %s", esp_err_to_name(err));
            }
        }

        gpio_isr_handler_add(gpio, gpio_isr_handler, this);
    }

    void STUSB4500Manager::task_main()
    {
        setup_interrupt(alert_gpio_);
        init_device();
        get_connection_status(OutputFormat::Log);
        get_active_pdo(OutputFormat::Log);
        
        while (true)
        {
            if (gpio_get_level(alert_gpio_) == 0 || ulTaskNotifyTake(pdTRUE, portMAX_DELAY))
            {
                if (gpio_get_level(alert_gpio_) == 0)
                {
                    ESP_LOGE(TAG, "Alert");
                    handle_alert();
                }
                
            }
        }
    }
};