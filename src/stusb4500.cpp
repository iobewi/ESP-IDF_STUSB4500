#include "config/stusb4500-config_macro.hpp"
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

    static const char *TAG = "STUSB4500";

    STUSB4500Manager::STUSB4500Manager(I2CDevices &i2c)
        : i2c_(i2c),
          cfg_(load_config_from_kconfig()),
          alert_gpio_(gpio_num_t(CONFIG_STUSB4500_INT_ALERT)),
          status_(i2c_)
          {}

    // === API PUBLIQUE ===

    void STUSB4500Manager::init()
    {
        xTaskCreatePinnedToCore(task_wrapper, "STUSB_Task", 4096, this, 5, &task_handle_, 1);
    }

    esp_err_t STUSB4500Manager::init_device()
    {
        RETURN_IF_ERROR(get_status());
        status_.alert_status_1_mask.decode(0);
        RETURN_IF_ERROR(status_.write_alert_status_mask());
        return ESP_OK;
    }

    esp_err_t STUSB4500Manager::check_config(Config &cfg)
    {
        NVMData new_nvm(cfg);
        Config active_cfg({}, {}, {});
        NVMData active_nvm(active_cfg);
        NVM iface_nvm(i2c_);
        RETURN_IF_ERROR(iface_nvm.read(active_nvm));
        if (! active_nvm.equals(new_nvm.to_array()))
        {
            iface_nvm.write(new_nvm);
            vTaskDelay(pdMS_TO_TICKS(1000));
            reset();
            active_nvm.print_diff(new_nvm.to_array());
            ESP_LOGI("STUSB4500", "Old configuration :");
            active_cfg.log();
            ESP_LOGI("STUSB4500", "New configuration :");
            cfg.log();
        }
        return ESP_OK;
    }

    esp_err_t STUSB4500Manager::handle_alert()
    {
        RETURN_IF_ERROR(status_.read_alert_status());

        if (status_.alert_status_1.port_status_al)
        {
            RETURN_IF_ERROR(status_.read_port_status_0());
            status_.port_status_0.log();
        }

        if (status_.alert_status_1.typec_monitoring_status_al)
        {
            RETURN_IF_ERROR(status_.read_typec_monitoring_status_0());
            status_.typec_monitoring_status_0.log();
        }

        if (status_.alert_status_1.cc_hw_fault_status_al)
        {
            RETURN_IF_ERROR(status_.read_cc_hw_fault_status_0());
            status_.cc_hw_fault_0.log();
        }

        if (status_.alert_status_1.prt_status_al)
        {
            RETURN_IF_ERROR(status_.read_prt_status());
            status_.prt_status.log();

            if (status_.prt_status.prl_hw_rst_received)
            {
                ESP_LOGW(TAG, "PD Hardware Reset detected. Clearing local PD state.");
            }

            if (status_.prt_status.prt_ibist_received)
            {
                ESP_LOGE(TAG, "PD BIST (Built-In Self Test) mode received! Unexpected behavior!");
            }

            if (status_.prt_status.prl_msg_received)
            {   
                get_active_pdo(OutputFormat::Log);
            }
        }
        return ESP_OK;
    }

    /// Envoie un soft reset au STUSB4500
    esp_err_t STUSB4500Manager::reset()
    {
        CTRL ctrl(i2c_);
        return ctrl.send_soft_reset();
    }

    /// Réécrit le PDO avec la configuration par défaut et force une renégociation
    esp_err_t STUSB4500Manager::reconfigure(uint8_t index, Config &cfg)
    {
        CTRL ctrl(i2c_);
        PDO active_pdo(i2c_,index,cfg.power().pdos[index]);
        RETURN_IF_ERROR(active_pdo.write());
        RETURN_IF_ERROR(ctrl.update_pdo_number(index));
        return ESP_OK;
    }

    /// Lit et retourne l’état courant de la connexion USB-C
    esp_err_t STUSB4500Manager::get_status(OutputFormat format)
    {
        RETURN_IF_ERROR(status_.get_status());
        HANDLE_OUTPUT(format, status_);
        return ESP_OK;
    }

    esp_err_t STUSB4500Manager::get_connection_status(OutputFormat format)
    {
        RETURN_IF_ERROR(status_.read_port_status_1());
        HANDLE_OUTPUT(format, status_.port_status_1);
        RETURN_IF_ERROR(status_.read_cc_status());
        HANDLE_OUTPUT(format, status_.cc_status);
        return ESP_OK;
    }

    esp_err_t STUSB4500Manager::get_active_pdo(OutputFormat format)
    {
        RXDatas rxdatas(i2c_);
        RETURN_IF_ERROR(rxdatas.read());
        RETURN_IF_ERROR(status_.read_policy_engine_state());
        HANDLE_OUTPUT(format, status_.policy_engine_state);
        if (status_.policy_engine_state.state == 0x18)
        {
            RDO rdo(i2c_);
            RETURN_IF_ERROR(rdo.read());
            uint8_t index = rdo.obj_position();
            if (index >= 1 && index <= cfg_.power().pdo_number)
            {
                HANDLE_OUTPUT(format, cfg_.power().pdos[index - 1]);
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
                ESP_LOGE("STUSB4500", "Failed to install ISR service: %s", esp_err_to_name(err));
            }
        }

        gpio_isr_handler_add(gpio, gpio_isr_handler, this);
    }

    void STUSB4500Manager::task_main()
    {
        setup_interrupt(alert_gpio_);
        init_device();
        check_config(cfg_);
        get_connection_status(OutputFormat::Log);
        get_active_pdo(OutputFormat::Log);
        while (true)
        {
            if (gpio_get_level(alert_gpio_) == 0 || ulTaskNotifyTake(pdTRUE, portMAX_DELAY))
            {
                handle_alert();
            }
        }
    }
};