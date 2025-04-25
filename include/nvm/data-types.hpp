#pragma once

#include <array>
#include <cstdint>
#include <cstring>

namespace stusb4500 {

    struct Bank0 {
        void decode(const uint8_t*) {}
        void encode(uint8_t*) const {}
    };

    struct Bank1 {
        uint8_t gpio_cfg;
        uint8_t vbus_dchg_mask;
        uint8_t vbus_disch_time_to_pdo;
        uint8_t discharge_time_to_0v;

        void decode(const uint8_t* buffer);
        void encode(uint8_t* buffer) const;
    };

    struct Bank2 {
        void decode(const uint8_t*) {}
        void encode(uint8_t*) const {}
    };

    struct Bank3 {
        bool snk_uncons_power;
        uint8_t dpm_snk_pdo_numb;
        bool usb_comm_capable;

        uint8_t lut_snk_pdo1_i;
        uint8_t snk_hl_pdo1;
        uint8_t snk_ll_pdo1;

        uint8_t lut_snk_pdo2_i;
        uint8_t snk_hl_pdo2;
        uint8_t snk_ll_pdo2;

        uint8_t lut_snk_pdo3_i;
        uint8_t snk_hl_pdo3;
        uint8_t snk_ll_pdo3;

        uint8_t snk_pdo_fill;

        void decode(const uint8_t* buffer);
        void encode(uint8_t* buffer) const;
    };

    struct Bank4 {
        uint16_t snk_pdo_flex1_v;
        uint16_t snk_pdo_flex2_v;
        uint16_t snk_pdo_flex_i;
        uint8_t power_ok_cfg;
        uint8_t spare;
        uint8_t req_src_current;
        uint8_t alert_status_mask;

        void decode(const uint8_t* buffer);
        void encode(uint8_t* buffer) const;
    };

    struct NVMData {
        Bank0 bank0;
        Bank1 bank1;
        Bank2 bank2;
        Bank3 bank3;
        Bank4 bank4;

        static constexpr size_t NVM_SIZE = 40;

        static constexpr std::array<uint8_t, NVM_SIZE> default_nvm_map = {
            0x00, 0x00, 0xB0, 0xAA, 0x00, 0x45, 0x00, 0x00,
            0x10, 0x40, 0x9C, 0x1C, 0xFF, 0x01, 0x3C, 0xDF,
            0x02, 0x40, 0x0F, 0x00, 0x32, 0x00, 0xFC, 0xF1,
            0x00, 0x19, 0x56, 0xAF, 0xF5, 0x35, 0x5F, 0x00,
            0x00, 0x4B, 0x90, 0x21, 0x00, 0x00, 0x00, 0xFB
        };
    };

} // namespace stusb4500
