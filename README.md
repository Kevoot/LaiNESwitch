LaiNES - Ported to Nintendo Switch by Kevoot
======
Ported from the original by [AndreaOrru](https://github.com/AndreaOrru/LaiNES) 
by [Kevoot](https://github.com/Kevoot)

Compact, cycle-accurate NES emulator in ~1000 lines of C++.

![File Browser](http://i.imgur.com/2tuDlCw.png)
![Super Mario Bros. 3](http://i.imgur.com/Gm4QWsE.png)
![Kirby's Adventure](http://i.imgur.com/xA2vwim.png)

![Star Wars](http://i.imgur.com/j3MmRba.png)
![Super Mario Bros.](http://i.imgur.com/yal0ps1.png)
![The Legend of Zelda](http://i.imgur.com/OLO02ij.png)

## How are lines counted?
There have been some discussions about how the number of lines has been calculated. The claim of ~1k lines doesn't include the libraries (boost and nes_apu). Future plans include eliminating those dependencies to make the code base even slimmer.

```
[andrea@manhattan src]$ cloc .
      24 text files.
      24 unique files.                              
       1 file ignored.

github.com/AlDanial/cloc v 1.70  T=0.03 s (780.3 files/s, 63170.2 lines/s)
-------------------------------------------------------------------------------
Language                     files          blank        comment           code
-------------------------------------------------------------------------------
C++                             11            210            110           1163
C/C++ Header                    12             87              7            285
-------------------------------------------------------------------------------
SUM:                            23            297            117           1448
-------------------------------------------------------------------------------
```

## Requirements
To Run:
    - A method of launching homebrew (NRO files)
To Build:
    - libnx + devkitpro [See](https://github.com/switchbrew/libnx);
    - From the pacman packages for libnx:
        -- sdl2, sdl2_image, sdl_gfx, sdl_mixer, jpg and png libs as well

## Building and running
- cd to the root directory and type `make`
- Put the resulting LaiNes.nro file on the Switch's SD card under /switch/LaiNes
- ROMs go in /switch/LaiNes/game

## Usage
The emulator comes bundled with a simple GUI to navigate the filesystem and set preferences. Use DPAD and 'A' to operate it. 'R' toggles between emulation and menu.

## Compatibility
LaiNES implements the most common mappers, which should be enough for a good percentage of the games:
- NROM (Mapper 000)
- MMC1 / SxROM (Mapper 001)
- UxROM (Mapper 002)
- CNROM (Mapper 003)
- MMC3, MMC6 / TxROM (Mapper 004)

You can check the compatibility for each ROM in the following list:
http://tuxnes.sourceforge.net/nesmapper.txt

## Technical notes
The 6502 CPU and the PPU are implemented in just 219 and 283 lines of code respectively.
Some clever meta-programming tricks are used to keep the codebase compact.
Here is a good example of how this is achieved:
```c++
/* Cycle emulation.
 *     For each CPU cycle, we call the PPU thrice, because it runs at 3 times the frequency. */
#define T   tick()
inline void tick() { PPU::step(); PPU::step(); PPU::step(); ... }
...

/* Addressing modes.
 *     These are all the possible ways instructions can access memory. */
typedef u16 (*Mode)(void);
inline u16 imm() { return PC++; }
...
inline u16 zpx() { T; return (zp() + X) % 0x100; }
...

/* Fetch parameter.
 *     Get the address of the opcode parameter in a, and the value in p. */
#define G  u16 a = m(); u8 p = rd(a)
...

/* Instruction emulation (LDx where x is in registers {A, X, Y}).
 *     upd_nz, not shown, just updates the CPU flags register. */
template<u8& r, Mode m> void ld() { G; upd_nz(r = p); }
...

/* Execute a CPU instruction.
 *     Opcodes are instantiated with the right template parameters
 *     (i.e. register and/or addressing mode).*/
void exec()
{
    switch (rd(PC++))  // Fetch the opcode.
    {
        // Select the right function to emulate the instruction:
         ...
         case 0xA0: return ld<Y,imm>();  case 0xA1: return ld<A,izx>();
         ...
    }
}
```

## Known issues
* The quality of the audio emulation on Linux is very poor. Typing `export SDL_AUDIODRIVER=ALSA` before running the emulator has been reported to improve it.

## Contributors
* [Jeff Katz](https://github.com/kraln) - Mapper 002 & 003, configuration.

## References and credits
- Original by [AndreaOrru](https://github.com/AndreaOrru/LaiNES)
- Special thanks to [Ulf Magnusson](https://github.com/ulfalizer) for the invaluable [PPU diagram] (http://wiki.nesdev.com/w/images/d/d1/Ntsc_timing.png) and for his [excellent implementation](https://github.com/ulfalizer/nesalizer) which was a big source of inspiration.
- blargg's APU sound chip emulator: http://blargg.8bitalley.com/libs/audio.html#Nes_Snd_Emu
- Complete hardware reference: http://problemkaputt.de/everynes.htm
- Tick-by-tick breakdown of 6502 instructions: http://nesdev.com/6502_cpu.txt

-- For Switch version:
[TurtleP](https://github.com/TurtleP) for reference material with SDL and listening to my whining.
