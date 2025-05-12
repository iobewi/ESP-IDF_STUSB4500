#include "config/stusb4500-config_macro.hpp"
#include "sdkconfig.h"

namespace stusb4500
{
    Config load_config_from_kconfig()
    {
        Config cfg({}, {}, {});
        // Discharge settings

#ifdef CONFIG_STUSB4500_DISCHARGE_DISABLE
        cfg.discharge().disable = (true);
#else
        cfg.discharge().disable = (false);
#endif

        cfg.discharge().time_to_0v = CONFIG_STUSB4500_DISCHARGE_TO_0V;
        cfg.discharge().time_to_pdo = CONFIG_STUSB4500_DISCHARGE_TO_PDO;

// Common flags
#ifdef CONFIG_STUSB4500_USB_COMM_CAPABLE
        cfg.power().usb_comm_capable = (true);
#else
        cfg.power().usb_comm_capable = (false);
#endif

#ifdef CONFIG_STUSB4500_DUAL_ROLE_POWER
        cfg.power().dual_role_power = (true);
#else
        cfg.power().dual_role_power = (false);
#endif

#ifdef CONFIG_STUSB4500_HIGHER_CAPABILITY
        cfg.power().higher_capability = (true);
#else
        cfg.power().higher_capability = (false);
#endif

#ifdef CONFIG_STUSB4500_UNCONSTRAINED_POWER
        cfg.power().unconstrained_power = (true);
#else
        cfg.power().unconstrained_power = (false);
#endif

#ifdef CONFIG_STUSB4500_REQ_SRC_CURRENT
        cfg.req_src_current = (true);
#else
        cfg.req_src_current = (false);
#endif

#ifdef CONFIG_STUSB4500_POWER_ONLY_5V
        cfg.power_only_5v = (true);
#else
        cfg.power_only_5v = (false);
#endif

        cfg.alert_mask = CONFIG_STUSB4500_ALERT_MASK;

#if CONFIG_STUSB4500_FRS_DEFAULT_USB
        cfg.power().frs = (FastRoleSwap::DefaultUSB);
#elif CONFIG_STUSB4500_FRS_1A5
        cfg.power().frs = (FastRoleSwap::A_1_5);
#elif CONFIG_STUSB4500_FRS_3A0
        cfg.power().frs = (FastRoleSwap::A_3_0);
#else
        cfg.power().frs = (FastRoleSwap::NotSupported);
#endif

// GPIO & autres
#if CONFIG_STUSB4500_GPIO_FUNC_SWCTRL
        cfg.gpio_function = GPIOFunction::SWCtrl;
#elif CONFIG_STUSB4500_GPIO_FUNC_DEBUG
        cfg.gpio_function = GPIOFunction::Debug;
#elif CONFIG_STUSB4500_GPIO_FUNC_SINK_POWER
        cfg.gpio_function = GPIOFunction::SinkPower;
#else
        cfg.gpio_function = GPIOFunction::ErrorRecovery;
#endif

#if CONFIG_STUSB4500_POWER_OK_CFG_1
        cfg.power_ok = PowerOkConfig::CONFIG_1;
#elif CONFIG_STUSB4500_POWER_OK_CFG_NOT_APPLICABLE
        cfg.power_ok = PowerOkConfig::NOT_APPLICABLE;
#elif CONFIG_STUSB4500_POWER_OK_CFG_3
        cfg.power_ok = PowerOkConfig::CONFIG_3;
#else
        cfg.power_ok = PowerOkConfig::CONFIG_2;
#endif

        // PDO1 : toujours actif
        cfg.power().pdos[0].voltage_mv = (5000);

#if defined(CONFIG_STUSB4500_PDO1_CURRENT_FLEX)
        cfg.power().pdos[0].current_ma = (0); // flexible = 0
#elif defined(CONFIG_STUSB4500_PDO1_CURRENT_500)
        cfg.power().pdos[0].current_ma = (500);
#elif defined(CONFIG_STUSB4500_PDO1_CURRENT_750)
        cfg.power().pdos[0].current_ma = (750);
#elif defined(CONFIG_STUSB4500_PDO1_CURRENT_1000)
        cfg.power().pdos[0].current_ma = (1000);
#elif defined(CONFIG_STUSB4500_PDO1_CURRENT_1250)
        cfg.power().pdos[0].current_ma = (1250);
#elif defined(CONFIG_STUSB4500_PDO1_CURRENT_1500)
        cfg.power().pdos[0].current_ma = (1500);
#elif defined(CONFIG_STUSB4500_PDO1_CURRENT_1750)
        cfg.power().pdos[0].current_ma = (1750);
#elif defined(CONFIG_STUSB4500_PDO1_CURRENT_2000)
        cfg.power().pdos[0].current_ma = (2000);
#elif defined(CONFIG_STUSB4500_PDO1_CURRENT_2250)
        cfg.power().pdos[0].current_ma = (2250);
#elif defined(CONFIG_STUSB4500_PDO1_CURRENT_2500)
        cfg.power().pdos[0].current_ma = (2500);
#elif defined(CONFIG_STUSB4500_PDO1_CURRENT_2750)
        cfg.power().pdos[0].current_ma = (2750);
#elif defined(CONFIG_STUSB4500_PDO1_CURRENT_3000)
        cfg.power().pdos[0].current_ma = (3000);
#elif defined(CONFIG_STUSB4500_PDO1_CURRENT_3500)
        cfg.power().pdos[0].current_ma = (3500);
#elif defined(CONFIG_STUSB4500_PDO1_CURRENT_4000)
        cfg.power().pdos[0].current_ma = (4000);
#elif defined(CONFIG_STUSB4500_PDO1_CURRENT_4500)
        cfg.power().pdos[0].current_ma = (4500);
#elif defined(CONFIG_STUSB4500_PDO1_CURRENT_5000)
        cfg.power().pdos[0].current_ma = (5000);
#endif
        cfg.power().pdos[0].vbus_monitor.lower_percent = CONFIG_STUSB4500_PDO1_VBUS_LOW;
        cfg.power().pdos[0].vbus_monitor.upper_percent = CONFIG_STUSB4500_PDO1_VBUS_HIGH;

        cfg.power().pdos[1].voltage_mv = (CONFIG_STUSB4500_PDO2_VOLTAGE);
#if defined(CONFIG_STUSB4500_PDO2_CURRENT_FLEX)
        cfg.power().pdos[1].current_ma = (0);
#elif defined(CONFIG_STUSB4500_PDO2_CURRENT_500)
        cfg.power().pdos[1].current_ma = (500);
#elif defined(CONFIG_STUSB4500_PDO2_CURRENT_750)
        cfg.power().pdos[1].current_ma = (750);
#elif defined(CONFIG_STUSB4500_PDO2_CURRENT_1000)
        cfg.power().pdos[1].current_ma = (1000);
#elif defined(CONFIG_STUSB4500_PDO2_CURRENT_1250)
        cfg.power().pdos[1].current_ma = (1250);
#elif defined(CONFIG_STUSB4500_PDO2_CURRENT_1500)
        cfg.power().pdos[1].current_ma = (1500);
#elif defined(CONFIG_STUSB4500_PDO2_CURRENT_1750)
        cfg.power().pdos[1].current_ma = (1750);
#elif defined(CONFIG_STUSB4500_PDO2_CURRENT_2000)
        cfg.power().pdos[1].current_ma = (2000);
#elif defined(CONFIG_STUSB4500_PDO2_CURRENT_2250)
        cfg.power().pdos[1].current_ma = (2250);
#elif defined(CONFIG_STUSB4500_PDO2_CURRENT_2500)
        cfg.power().pdos[1].current_ma = (2500);
#elif defined(CONFIG_STUSB4500_PDO2_CURRENT_2750)
        cfg.power().pdos[1].current_ma = (2750);
#elif defined(CONFIG_STUSB4500_PDO2_CURRENT_3000)
        cfg.power().pdos[1].current_ma = (3000);
#elif defined(CONFIG_STUSB4500_PDO2_CURRENT_3500)
        cfg.power().pdos[1].current_ma = (3500);
#elif defined(CONFIG_STUSB4500_PDO2_CURRENT_4000)
        cfg.power().pdos[1].current_ma = (4000);
#elif defined(CONFIG_STUSB4500_PDO2_CURRENT_4500)
        cfg.power().pdos[1].current_ma = (4500);
#elif defined(CONFIG_STUSB4500_PDO2_CURRENT_5000)
        cfg.power().pdos[1].current_ma = (5000);
#endif
        cfg.power().pdos[1].vbus_monitor.lower_percent = CONFIG_STUSB4500_PDO2_VBUS_LOW;
        cfg.power().pdos[1].vbus_monitor.upper_percent = CONFIG_STUSB4500_PDO2_VBUS_HIGH;

        cfg.power().pdos[2].voltage_mv = (CONFIG_STUSB4500_PDO3_VOLTAGE);
#if defined(CONFIG_STUSB4500_PDO3_CURRENT_FLEX)
        cfg.power().pdos[2].current_ma = (0);
#elif defined(CONFIG_STUSB4500_PDO3_CURRENT_500)
        cfg.power().pdos[2].current_ma = (500);
#elif defined(CONFIG_STUSB4500_PDO3_CURRENT_750)
        cfg.power().pdos[2].current_ma = (750);
#elif defined(CONFIG_STUSB4500_PDO3_CURRENT_1000)
        cfg.power().pdos[2].current_ma = (1000);
#elif defined(CONFIG_STUSB4500_PDO3_CURRENT_1250)
        cfg.power().pdos[2].current_ma = (1250);
#elif defined(CONFIG_STUSB4500_PDO3_CURRENT_1500)
        cfg.power().pdos[2].current_ma = (1500);
#elif defined(CONFIG_STUSB4500_PDO3_CURRENT_1750)
        cfg.power().pdos[2].current_ma = (1750);
#elif defined(CONFIG_STUSB4500_PDO3_CURRENT_2000)
        cfg.power().pdos[2].current_ma = (2000);
#elif defined(CONFIG_STUSB4500_PDO3_CURRENT_2250)
        cfg.power().pdos[2].current_ma = (2250);
#elif defined(CONFIG_STUSB4500_PDO3_CURRENT_2500)
        cfg.power().pdos[2].current_ma = (2500);
#elif defined(CONFIG_STUSB4500_PDO3_CURRENT_2750)
        cfg.power().pdos[2].current_ma = (2750);
#elif defined(CONFIG_STUSB4500_PDO3_CURRENT_3000)
        cfg.power().pdos[2].current_ma = (3000);
#elif defined(CONFIG_STUSB4500_PDO3_CURRENT_3500)
        cfg.power().pdos[2].current_ma = (3500);
#elif defined(CONFIG_STUSB4500_PDO3_CURRENT_4000)
        cfg.power().pdos[2].current_ma = (4000);
#elif defined(CONFIG_STUSB4500_PDO3_CURRENT_4500)
        cfg.power().pdos[2].current_ma = (4500);
#elif defined(CONFIG_STUSB4500_PDO3_CURRENT_5000)
        cfg.power().pdos[2].current_ma = (5000);
#endif
        cfg.power().pdos[2].vbus_monitor.lower_percent = CONFIG_STUSB4500_PDO3_VBUS_LOW;
        cfg.power().pdos[2].vbus_monitor.upper_percent = CONFIG_STUSB4500_PDO3_VBUS_HIGH;

#if CONFIG_STUSB4500_PDO3_ENABLE
        cfg.power().pdo_number = 3;
        cfg.power().pdos[0].defined = true;
        cfg.power().pdos[1].defined = true;
        cfg.power().pdos[2].defined = true;
#elif CONFIG_STUSB4500_PDO2_ENABLE
        cfg.power().pdo_number = 2;
        cfg.power().pdos[0].defined = true;
        cfg.power().pdos[1].defined = true;
        cfg.power().pdos[2].defined = false;
#else
        cfg.power().pdo_number = 1;
        cfg.power().pdos[0].defined = true;
        cfg.power().pdos[1].defined = false;
        cfg.power().pdos[2].defined = false;
#endif
        // Flex current
        cfg.power().flex_current_ma = CONFIG_STUSB4500_FLEX_CURRENT;

        return cfg;
    }
};