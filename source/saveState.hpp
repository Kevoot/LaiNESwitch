#pragma once
#include <string>
#include <switch.h>
#include <jansson.h>

#include "common.hpp"
#include "cpu.hpp"
#include "ppu.hpp"

namespace SaveState
{

class SaveState
{
  public:
    CPU::CpuSave * cpu;
    PPU::PpuSave * ppu;

    CreateState();
    RestoreState();
};

} // namespace SaveState
