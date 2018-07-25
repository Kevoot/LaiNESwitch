#pragma once
#include "common.hpp"

namespace Cartridge {


template <bool wr>
uint8_t access(uint16_t addr, uint8_t v = 0);

template <bool wr>
uint8_t chr_access(uint16_t addr, uint8_t v = 0);

void signal_scanline();
int load(const char* fileName);
bool loaded();


}
