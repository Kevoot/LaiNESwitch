#pragma once
#include "mapper.hpp"


class Mapper4 : public Mapper
{
    uint8_t reg8000;
    uint8_t regs[8];
    bool horizMirroring;

    uint8_t irqPeriod;
    uint8_t irqCounter;
    bool irqEnabled;

    void apply();

  public:
    Mapper4(uint8_t* rom) : Mapper(rom)
    {
        for (int i = 0; i < 8; i++)
            regs[i] = 0;

        horizMirroring = true;
        irqEnabled = false;
        irqPeriod = irqCounter = 0;

        map_prg<8>(3, -1);
        apply();
    }

    uint8_t write(uint16_t addr, uint8_t v);
    uint8_t chr_write(uint16_t addr, uint8_t v);

    void signal_scanline();
};
