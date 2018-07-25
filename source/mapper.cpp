#include "ppu.hpp"
#include "mapper.hpp"


Mapper::Mapper(uint8_t* rom) : rom(rom)
{
    // Read infos from header:
    prgSize      = rom[4] * 0x4000;
    chrSize      = rom[5] * 0x2000;
    prgRamSize   = rom[8] ? rom[8] * 0x2000 : 0x2000;
    set_mirroring((rom[6] & 1) ? PPU::VERTICAL : PPU::HORIZONTAL);

    this->prg    = rom + 16;
    this->prgRam = new uint8_t[prgRamSize];

    // CHR ROM:
    if (chrSize)
        this->chr = rom + 16 + prgSize;
    // CHR RAM:
    else
    {
        chrRam = true;
        chrSize = 0x2000;
        this->chr = new uint8_t[chrSize];
    }
}

Mapper::~Mapper()
{
    delete rom;
    delete prgRam;
    if (chrRam)
        delete chr;
}

/* Access to memory */
uint8_t Mapper::read(uint16_t addr)
{
    if (addr >= 0x8000)
        return prg[prgMap[(addr - 0x8000) / 0x2000] + ((addr - 0x8000) % 0x2000)];
    else
        return prgRam[addr - 0x6000];
}

uint8_t Mapper::chr_read(uint16_t addr)
{
    return chr[chrMap[addr / 0x400] + (addr % 0x400)];
}

/* PRG mapping functions */
template <int pageKBs> void Mapper::map_prg(int slot, int bank)
{
    if (bank < 0)
        bank = (prgSize / (0x400*pageKBs)) + bank;

    for (int i = 0; i < (pageKBs/8); i++)
        prgMap[(pageKBs/8) * slot + i] = (pageKBs*0x400*bank + 0x2000*i) % prgSize;
}
template void Mapper::map_prg<32>(int, int);
template void Mapper::map_prg<16>(int, int);
template void Mapper::map_prg<8> (int, int);

/* CHR mapping functions */
template <int pageKBs> void Mapper::map_chr(int slot, int bank)
{
    for (int i = 0; i < pageKBs; i++)
        chrMap[pageKBs*slot + i] = (pageKBs*0x400*bank + 0x400*i) % chrSize;
}
template void Mapper::map_chr<8>(int, int);
template void Mapper::map_chr<4>(int, int);
template void Mapper::map_chr<2>(int, int);
template void Mapper::map_chr<1>(int, int);
