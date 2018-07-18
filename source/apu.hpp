#pragma once
#include "common.hpp"

namespace APU {


template <bool write> uint8_t access(int elapsed, uint16_t addr, uint8_t v = 0);
void run_frame(int elapsed);
void reset();
void init();


}
