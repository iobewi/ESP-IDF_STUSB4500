#pragma once
#include <cstdint>
#include "config/config.hpp"
#include "pd/pdo.hpp"

namespace stusb4500
{
    class Bank
    {
    public:
        explicit Bank(Config &data)
            : data_(data) {}
        virtual void decode(const uint8_t *buffer) = 0;
        virtual void encode(uint8_t *buffer) const = 0;
        virtual ~Bank() = default;
    protected:
    Config &data_;
    };

    class Bank0 : public Bank
    {
    public:
        explicit Bank0(Config &data) : Bank(data) {}

        void decode(const uint8_t * /*buffer*/) override {}
        void encode(uint8_t * /*buffer*/) const override {}
    };

    class Bank1 : public Bank
    {
    public:
        explicit Bank1(Config &data) : Bank(data) {}
 
        void decode(const uint8_t *buffer) override;
        void encode(uint8_t *buffer) const override;
    };

    class Bank2 : public Bank
    {
    public:
        explicit Bank2(Config &data) : Bank(data) {}

        void decode(const uint8_t * /*buffer*/) override {}
        void encode(uint8_t * /*buffer*/) const override {}
    };

    class Bank3 : public Bank
    {
    public:
        explicit Bank3(Config &data) : Bank(data) {}

        void decode(const uint8_t *buffer);
        void encode(uint8_t *buffer) const;
    };

    class Bank4 : public Bank
    {
    public:
        explicit Bank4(Config &data) : Bank(data) {}

        void decode(const uint8_t *buffer);
        void encode(uint8_t *buffer) const;
    };
};