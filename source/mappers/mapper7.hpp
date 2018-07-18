#pragma once
#include "mapper.hpp"
#include "ppu.hpp"

class Mapper7 : public Mapper
{
    uint8_t regs[1];
    void apply();

  public:
    Mapper7(uint8_t* rom) : Mapper(rom)
    {
        regs[0] = 0;
        apply();
    }

    uint8_t write(uint16_t addr, uint8_t v);
    uint8_t chr_write(uint16_t addr, uint8_t v);
};