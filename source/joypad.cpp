#include "gui.hpp"
#include "menu.hpp"

namespace Joypad {


uint8_t joypad_bits[2];  // Joypad shift registers.
bool strobe;        // Joypad strobe latch.

/* Read joypad state (NES register format) */
uint8_t read_state(int8_t n)
{
    // When strobe is high, it keeps reading A:
    if (strobe)
        return 0x40 | (GUI::get_joypad_state(n) & 1);

    // Get the status of a button and shift the register:
    uint8_t j = 0x40 | (joypad_bits[n] & 1);
    joypad_bits[n] = 0x80 | (joypad_bits[n] >> 1);
    return j;
}

void write_strobe(bool v)
{
    // Read the joypad data on strobe's transition 1 -> 0.
    if (strobe and !v)
        for (int i = 0; i < 2; i++)
            joypad_bits[i] = GUI::get_joypad_state(i);

    strobe = v;
}


}
