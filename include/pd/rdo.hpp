#pragma once

#include <cstdint>

#include "esp_err.h"
#include "interface.hpp"

namespace stusb4500
{

    class RDO : public INTERFACE
    {
    public:
        explicit RDO(I2CDevices &dev) : INTERFACE(dev) {}

        esp_err_t read();
        
        void decode(const uint8_t *buf, size_t len);
        uint32_t encode() const;

        uint8_t obj_position() const;
        bool giveback() const;
        bool capability_mismatch() const;
        bool usb_comm_capable() const;
        bool no_usb_suspend() const;
        bool unchunked_ext() const;
        uint16_t operating_ma() const;
        uint16_t max_operating_ma() const;
        
        void log() const;

    private:
        uint32_t raw_ = 0;

        static constexpr uint8_t reg_addr = 0x91;
        static constexpr uint8_t reg_len = 4;
    };

}
