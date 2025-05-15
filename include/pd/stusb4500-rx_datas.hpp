#pragma once

#include <cstdint>
#include "stusb4500-interface.hpp"
#include "pd/stusb4500-pdo.hpp"

namespace stusb4500 {

class RXDatas : public INTERFACE {
public:
    explicit RXDatas(I2CDevices& dev) : INTERFACE(dev) {}

    esp_err_t read();

    void decode(const uint8_t* buf, size_t len);

    uint16_t header() const;
    uint8_t message_type() const;
    uint8_t num_objects() const;
    bool is_data_message() const;
    bool is_control_message() const;

    PowerProfile get_pdo(size_t index) const;

    void log() const;

private:
    inline static const char *TAG = "STUSB4500-RXDATAS";
    uint32_t data_[7] = {0};
    uint16_t header_ = 0;

    static constexpr uint8_t reg_addr = 0x31;
    static constexpr uint8_t reg_len = 30;
};

}
