#pragma once
#include "mapper.hpp"


class Mapper1 : public Mapper
{
    int writeN;
    uint8_t tmpReg;
    uint8_t regs[4];

    void apply();

  public:
    Mapper1(uint8_t* rom) : Mapper(rom)
    {
        regs[0] = 0x0C;
        writeN = tmpReg = regs[1] = regs[2] = regs[3] = 0;
        apply();
    }

    uint8_t write(uint16_t addr, uint8_t v);
    uint8_t chr_write(uint16_t addr, uint8_t v);
};
