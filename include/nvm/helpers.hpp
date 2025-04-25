#pragma once

#include <array>
#include <vector>
#include <tuple>
#include "config-types.hpp"
#include "data-types.hpp"

namespace stusb4500 {

    // Taille totale de la NVM
    constexpr size_t NVM_SIZE = 40;

    // Fonctions de décodage/encodage global
    void decode_all(NVMData& nvm, const uint8_t* buffer);
    void encode_all(const NVMData& nvm, uint8_t* buffer);

    // Conversion en tableau
    std::array<uint8_t, NVM_SIZE> to_array(const NVMData& nvm);

    // Comparaison
    bool equals(const NVMData& nvm, const std::array<uint8_t, NVM_SIZE>& other);
    std::vector<std::tuple<size_t, uint8_t, uint8_t>> diff(const NVMData& nvm, const std::array<uint8_t, NVM_SIZE>& other);
    void print_diff(const NVMData& nvm, const std::array<uint8_t, NVM_SIZE>& other);

    // Liaison avec la config système
    ConfigSYS getconfig(const NVMData& nvm);
    void apply_config(NVMData& nvm, const ConfigSYS& config);
    void dump_config(const ConfigSYS& config);
}
