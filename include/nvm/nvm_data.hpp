#pragma once

#include <vector>
#include <array>
#include <cstring>

#include "nvm/banks.hpp"

namespace stusb4500
{
    
    class NVMData
    {
    public:
        Bank0 bank0;
        Bank1 bank1;
        Bank2 bank2;
        Bank3 bank3;
        Bank4 bank4;

        explicit NVMData(Config &data)
            : bank0(data),
              bank1(data),
              bank2(data),
              bank3(data),
              bank4(data)
        {}

        static const std::array<uint8_t, 40> default_nvm_map;

        void decode(const uint8_t *buffer);

        void encode(uint8_t *buffer) const;

        std::array<uint8_t, 40> to_array() const;

        bool equals(const std::array<uint8_t, 40> &other) const;

        std::vector<std::tuple<size_t, uint8_t, uint8_t>> diff(const std::array<uint8_t, 40> &other) const;

        void print_diff(const std::array<uint8_t, 40> &other) const;

        void log() const;

    };
} // namespace stusb4500
