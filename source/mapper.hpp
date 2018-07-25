#pragma once
#include <cstring>
#include "common.hpp"


class Mapper
{
    uint8_t* rom;
    bool chrRam = false;

  protected:
    uint32_t prgMap[4];
    uint32_t chrMap[8];

    uint8_t *prg, *chr, *prgRam;
    uint32_t prgSize, chrSize, prgRamSize;

    template <int pageKBs> void map_prg(int slot, int bank);
    template <int pageKBs> void map_chr(int slot, int bank);

  public:
    Mapper(uint8_t* rom);
    ~Mapper();

    uint8_t read(uint16_t addr);
    virtual uint8_t write(uint16_t addr, uint8_t v) { return v; }

    uint8_t chr_read(uint16_t addr);
    virtual uint8_t chr_write(uint16_t addr, uint8_t v) { return v; }

    virtual void signal_scanline() {}
};
