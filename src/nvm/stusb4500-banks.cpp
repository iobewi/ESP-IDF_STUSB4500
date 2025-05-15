#include <cstdint>
#include "nvm/stusb4500-banks.hpp"

namespace stusb4500
{
    uint8_t pack4(uint8_t low, uint8_t high)
    {
        return (low & 0x0F) | ((high & 0x0F) << 4);
    }

     uint8_t extract_low4(uint8_t value)
    {
        return value & 0x0F;
    }

     uint8_t extract_high4(uint8_t value)
    {
        return (value >> 4) & 0x0F;
    }

    uint16_t encode_voltage_step(uint16_t voltage_mv)
    {
        return voltage_mv / 50;
    }

    uint16_t decode_voltage_step(uint16_t step)
    {
        return step * 50;
    }


    uint16_t encode_current_step(uint16_t current_mv)
    {
        return current_mv / 10;
    }

    uint16_t decode_current_step(uint16_t step)
    {
        return step * 10;
    }

   constexpr uint16_t decode_lup_current(uint8_t step)
    {
        constexpr uint16_t current_table[16] = {
            0, 500, 750, 1000,
            1250, 1500, 1750, 2000,
            2250, 2500, 2750, 3000,
            3500, 4000, 4500, 5000
        };
        return current_table[step & 0x0F];
    }


    constexpr uint8_t encode_lup_current(uint16_t current_ma)
    {
        constexpr uint16_t current_table[16] = {
            0, 500, 750, 1000,
            1250, 1500, 1750, 2000,
            2250, 2500, 2750, 3000,
            3500, 4000, 4500, 5000
        };

        for (uint8_t i = 0; i < 16; ++i)
        {
            if (current_table[i] == current_ma)
                return i;
        }

        return 0x00;
    }


    void Bank1::decode(const uint8_t *buffer)
    {
        data_.gpio_function = static_cast<ConfigParams::GPIOFunction>((buffer[0] >> 4) & 0x03);
        data_.discharge_.time_to_pdo = extract_low4(buffer[2]);
        data_.discharge_.time_to_0v = extract_high4(buffer[2]);
    }

    void Bank1::encode(uint8_t *buffer) const
    {
        buffer[0] = (buffer[0] & 0xCF) | ((static_cast<uint8_t>(data_.gpio_function) & 0x03) << 4);
        buffer[2] = pack4(data_.discharge_.time_to_pdo, data_.discharge_.time_to_0v);
    }

    void Bank3::decode(const uint8_t *buffer)
    {
        auto &power = data_.power_;
        // Octet 2

        power.usb_comm_capable = (buffer[2] & 0x01);
        power.pdo_number = (buffer[2] >> 1) & 0x03;
        power.unconstrained_power = ((buffer[2] >> 3) & 0x01);
        power.pdos[0].current_ma = (decode_lup_current(extract_high4(buffer[2])));
        // Octet 3
        power.pdos[0].vbus_monitor.lower_percent = (extract_low4(buffer[3]));
        power.pdos[0].vbus_monitor.upper_percent = (extract_high4(buffer[3]));
        // Octet 4
        power.pdos[1].current_ma = (decode_lup_current(extract_low4(buffer[4])));
        power.pdos[1].vbus_monitor.lower_percent = (extract_high4(buffer[4]));

        // Octet 5
        power.pdos[1].vbus_monitor.upper_percent = (extract_low4(buffer[5]));
        power.pdos[2].current_ma = (decode_lup_current(extract_high4(buffer[5])));

        // Octet 6
        power.pdos[2].vbus_monitor.lower_percent = (extract_low4(buffer[6]));
        power.pdos[2].vbus_monitor.upper_percent = (extract_high4(buffer[6]));
    }

    void Bank3::encode(uint8_t *buffer) const
    {
        auto &power = data_.power_;
        
        buffer[2] = (power.usb_comm_capable & 0x01) | ((power.pdo_number & 0x03) << 1) | ((power.unconstrained_power & 0x01) << 3) | ((encode_lup_current(power.pdos[0].current_ma) & 0x0F )<< 4);

        buffer[3] = pack4(power.pdos[0].vbus_monitor.lower_percent,
            power.pdos[0].vbus_monitor.upper_percent);

        buffer[4] = pack4(encode_lup_current(power.pdos[1].current_ma),
        power.pdos[1].vbus_monitor.lower_percent);

        buffer[5] = pack4(power.pdos[1].vbus_monitor.upper_percent,
                          encode_lup_current(power.pdos[2].current_ma));

        buffer[6] = pack4(power.pdos[2].vbus_monitor.lower_percent,
                          power.pdos[2].vbus_monitor.upper_percent);
    }

    void Bank4::decode(const uint8_t *buffer)
    {
        auto &power = data_.power_;

        power.pdos[1].voltage_mv = decode_voltage_step(((buffer[0] >> 6) & 0x03) | (buffer[1] << 2));
        power.pdos[2].voltage_mv = decode_voltage_step((buffer[2] | ((buffer[3] & 0x03) << 8)));
        power.flex_current_ma = decode_current_step(((buffer[3] >> 2) & 0x3F) | ((buffer[4] & 0x0F) << 6));
        data_.power_ok = static_cast<ConfigParams::PowerOkConfig>((buffer[4] >> 5) & 0x03);
        data_.req_src_current = (buffer[6] >> 4) & 0x01;
        data_.alert_mask.set_raw(buffer[7]);
    }

    void Bank4::encode(uint8_t *buffer) const
    {
        auto &power = data_.power_;

        buffer[0] = (buffer[0] & 0x3F) | ((encode_voltage_step(power.pdos[1].voltage_mv) & 0x03) << 6);
        buffer[1] = (encode_voltage_step(power.pdos[1].voltage_mv) >> 2) & 0xFF;
        buffer[2] = encode_voltage_step(power.pdos[2].voltage_mv) & 0xFF;
        buffer[3] = ((encode_voltage_step(power.pdos[2].voltage_mv) >> 8) & 0x03) | ((encode_current_step(power.flex_current_ma) & 0x3F) << 2);
        buffer[4] = ((encode_current_step(power.flex_current_ma) >> 6) & 0x0F) | ((static_cast<uint8_t>(data_.power_ok) & 0x03) << 5);
        buffer[6] = (buffer[6] & 0xEF) | ((data_.req_src_current & 0x01) << 4);
    }
};