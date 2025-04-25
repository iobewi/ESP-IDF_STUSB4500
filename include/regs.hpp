#pragma once

#include <cstdint>
#include <cstring>

namespace stusb4500 {

    // === Status Registers ===
    constexpr uint8_t REG_ALERT_STATUS_1         = 0x0B;
    constexpr uint8_t REG_ALERT_STATUS_1_MASK    = 0x0C;
    constexpr uint8_t REG_PORT_STATUS_0          = 0x0D;
    constexpr uint8_t REG_PORT_STATUS_1          = 0x0E;
    constexpr uint8_t REG_TYPEC_MONITORING_0     = 0x0F;
    constexpr uint8_t REG_TYPEC_MONITORING_1     = 0x10;
    constexpr uint8_t REG_CC_STATUS              = 0x11;
    constexpr uint8_t REG_CC_HW_FAULT_0          = 0x12;
    constexpr uint8_t REG_CC_HW_FAULT_1          = 0x13;
    constexpr uint8_t REG_PD_TYPEC_STATUS        = 0x14;
    constexpr uint8_t REG_TYPEC_STATUS           = 0x15;
    constexpr uint8_t REG_PRT_STATUS             = 0x16;
    constexpr uint8_t REG_PD_COMMAND_CTRL        = 0x1A;
    constexpr uint8_t REG_PE_FSM                 = 0x29;
    constexpr uint8_t REG_RDO_REG_STATUS_0       = 0x91;
    constexpr uint8_t REG_RDO_REG_STATUS_1       = 0x92;
    constexpr uint8_t REG_RDO_REG_STATUS_2       = 0x93;
    constexpr uint8_t REG_RDO_REG_STATUS_3       = 0x94;
    constexpr uint8_t REG_RX_HEADER_LOW          = 0x31;
    constexpr uint8_t REG_RX_HEADER_HIGH         = 0x32;
    constexpr uint8_t REG_DPM_PDO_NUMB           = 0x70;

    // === RX Data Object Registers (PDO reçus) ===
    constexpr uint8_t REG_RX_DATA_OBJ_START      = 0x33; // RX_DATA_OBJ1_0
    constexpr uint8_t REG_RX_DATA_OBJ_END        = 0x4E; // RX_DATA_OBJ7_3
    constexpr size_t  RX_DATA_OBJ_COUNT          = (REG_RX_DATA_OBJ_END - REG_RX_DATA_OBJ_START + 1) / 4;

    // === TX Header and Message ===
    constexpr uint8_t REG_TX_HEADER_LOW          = 0x51;
    constexpr uint8_t REG_TX_HEADER_HIGH         = 0x52;

    // === Sink PDO Registers ===
    constexpr uint8_t REG_DPM_SNK_PDO_START      = 0x85;
    constexpr uint8_t REG_DPM_SNK_PDO_END        = 0x90;
    constexpr size_t  SINK_PDO_REG_COUNT         = (REG_DPM_SNK_PDO_END - REG_DPM_SNK_PDO_START + 1) / 4;

    // === RDO Status Registers ===
    constexpr uint8_t REG_RDO_STATUS_START       = 0x91;
    constexpr uint8_t REG_RDO_STATUS_END         = 0x94;
    constexpr size_t  RDO_STATUS_REG_COUNT       = REG_RDO_STATUS_END - REG_RDO_STATUS_START + 1;

} // namespace stusb4500
