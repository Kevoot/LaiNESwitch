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
#include "ppu.hpp"
#include "cartridge.hpp"

namespace Cartridge {


Mapper* mapper = nullptr;  // Mapper chip.

/* PRG-ROM access */
template <bool wr> u8 access(u16 addr, u8 v)
{
    if (!wr) return mapper->read(addr);
    else     return mapper->write(addr, v);
}
template u8 access<0>(u16, u8); template u8 access<1>(u16, u8);

/* CHR-ROM/RAM access */
template <bool wr> u8 chr_access(u16 addr, u8 v)
{
    if (!wr) return mapper->chr_read(addr);
    else     return mapper->chr_write(addr, v);
}
template u8 chr_access<0>(u16, u8); template u8 chr_access<1>(u16, u8);

void signal_scanline()
{
    mapper->signal_scanline();
}

/* Load the ROM from a file. */
int load(const char* fileName)
{
    FILE* f = fopen(fileName, "rb");

    if(f == NULL) {
        printf("FILE open operation failure!\n");
        return -1;
    }
    printf("FILE operation succeeded!\n");

    fseek(f, 0, SEEK_END);
    int size = ftell(f);

    fseek(f, 0, SEEK_SET);

    u8* rom = new u8[size];
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
    }

    printf("Initial Mapping completed, beginning CPU power on\n");
    CPU::power();
    printf("CPU power on completed, resetting PPU\n");
    PPU::reset();
    printf("PPU reset, resetting APU\n");
    APU::reset();
    printf("APU reset complete\n");

    return 0;
}

bool loaded()
{
    return mapper != nullptr;
}


}
