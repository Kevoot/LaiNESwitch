#pragma once
#include "mapper.hpp"


class Mapper3 : public Mapper
{
    uint8_t regs[1];
    bool vertical_mirroring;
    bool PRG_size_16k;
    void apply();

    public:
    Mapper3(uint8_t* rom) : Mapper(rom)
    {
        PRG_size_16k = rom[4] == 1;
        vertical_mirroring = rom[6] & 0x01;
        regs[0] = 0;
        apply();
    }

    uint8_t write(uint16_t addr, uint8_t v);
    uint8_t chr_write(uint16_t addr, uint8_t v);
};

