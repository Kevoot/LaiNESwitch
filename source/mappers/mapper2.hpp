#pragma once
#include "mapper.hpp"


class Mapper2 : public Mapper
{
    uint8_t regs[1];
    bool vertical_mirroring;

    void apply();

  public:
    Mapper2(uint8_t* rom) : Mapper(rom)
    {
        regs[0] = 0;
        vertical_mirroring = rom[6] & 0x01;
        apply();
    }

    uint8_t write(uint16_t addr, uint8_t v);
    uint8_t chr_write(uint16_t addr, uint8_t v);
};
