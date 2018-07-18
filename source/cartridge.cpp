#include <cstdio>
#include <csignal>
#include <switch.h>
#include <SDL2/SDL.h>
#include "apu.hpp"
#include "cpu.hpp"
#include "mappers/mapper0.hpp"
#include "mappers/mapper1.hpp"
#include "mappers/mapper2.hpp"
#include "mappers/mapper3.hpp"
#include "mappers/mapper4.hpp"
#include "mappers/mapper7.hpp"
#include "ppu.hpp"
#include "cartridge.hpp"

namespace Cartridge {


Mapper* mapper = nullptr;  // Mapper chip.

/* PRG-ROM access */
template <bool wr> uint8_t access(uint16_t addr, uint8_t v)
{
    if (!wr) return mapper->read(addr);
    else     return mapper->write(addr, v);
}

template uint8_t access<0>(uint16_t, uint8_t); 
template uint8_t access<1>(uint16_t, uint8_t);

/* CHR-ROM/RAM access */
template <bool wr> uint8_t chr_access(uint16_t addr, uint8_t v)
{
    if (!wr) return mapper->chr_read(addr);
    else     return mapper->chr_write(addr, v);
}

template uint8_t chr_access<0>(uint16_t, uint8_t);
template uint8_t chr_access<1>(uint16_t, uint8_t);

void signal_scanline()
{
    mapper->signal_scanline();
}

/* Load the ROM from a file. */
int load(const char* fileName)
{
    FILE* f = fopen(fileName, "rb");

    if(f == NULL) {
        return -1;
    }

    fseek(f, 0, SEEK_END);
    int size = ftell(f);

    fseek(f, 0, SEEK_SET);

    uint8_t* rom = new uint8_t[size];
    fread(rom, size, 1, f);
    fclose(f);

    int mapperNum = (rom[7] & 0xF0) | (rom[6] >> 4);
    if (loaded()) delete mapper;
    switch (mapperNum)
    {
        case 0:  mapper = new Mapper0(rom); break;
        case 1:  mapper = new Mapper1(rom); break;
        case 2:  mapper = new Mapper2(rom); break;
        case 3:  mapper = new Mapper3(rom); break;
        case 4:  mapper = new Mapper4(rom); break;
        case 7:  mapper = new Mapper7(rom); break;
    }

    CPU::power();
    PPU::reset();
    APU::reset();

    return 0;
}

bool loaded()
{
    return mapper != nullptr;
}


}
