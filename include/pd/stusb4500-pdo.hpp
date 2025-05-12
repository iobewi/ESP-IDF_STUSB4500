#pragma once
#include "stusb4500-interface.hpp"
#include "stusb4500-common_types.hpp"

namespace stusb4500
{
    class PDO : public INTERFACE
    {
    public:
        explicit PDO(I2CDevices &dev,
            const size_t index = 2,
            const PDObjectProfile& pdo_profile = {});
            
        esp_err_t read();
        esp_err_t write();

        // Accès aux blocs
        PowerProfile &power() { return power_; }
        const PowerProfile &power() const { return power_; }

    private:
        size_t index_;
        PowerProfile power_;
        static constexpr uint8_t base_reg_addr = 0x85;
        static constexpr uint8_t reg_len = 4;

    };
};
