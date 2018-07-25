#pragma once
#include "common.hpp"

namespace Joypad
{

uint8_t read_state(int8_t n);
void write_strobe(bool v);

} // namespace Joypad
