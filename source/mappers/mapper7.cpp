#include "ppu.hpp"
#include "mappers/mapper7.hpp"

/* Based off of https://wiki.nesdev.com/w/index.php/AxROM */

/* Apply the registers state */
void Mapper7::apply()
{
   /*
    * 32 kb PRG ROM Banks
    * 0x8000 - 0xFFFF swappable
    */
    map_prg<32>(0, regs[0] & 0x7);

    /* 8k of CHR (ram) */
    map_chr<8>(0, 0);

    /* Mirroring based on bit 5 */
    set_mirroring((regs[0] & 0x10) ? PPU::ONE_SCREEN_HI : PPU::ONE_SCREEN_LO);
}

uint8_t Mapper7::write(uint16_t addr, uint8_t v)
{
    /* check for bus contingency? (addr & 0x8000 == v?) nah */

    /* bank switching */
    if (addr & 0x8000)
    {
        regs[0] = v;
        apply();
    }
    return v;
}

uint8_t Mapper7::chr_write(uint16_t addr, uint8_t v)
{
    return chr[addr] = v;
}