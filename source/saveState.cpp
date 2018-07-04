#include "saveState.hpp"

namespace SaveState {
    SaveState::CreateState() {
        this->cpu = new CPU::CpuSave();
        this->ppu = new PPU::PpuSave();
    }

    SaveState::RestoreState() {
        CPU::restoreCpuState(this->cpu);
        // PPU::restorePpuState(this->ppu);
    }
}