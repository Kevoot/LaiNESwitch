#include <cstdlib>
#include <cstring>
#include <iostream>
#include "apu.hpp"
#include "cartridge.hpp"
#include "joypad.hpp"
#include "ppu.hpp"
#include "cpu.hpp"
#include <exception>

namespace CPU
{

/* CPU state */
uint8_t ram[0x800];
uint8_t A, X, Y, S;
uint16_t PC;
Flags P;
bool nmi, irq;

// Remaining clocks to end frame:
const int TOTAL_CYCLES = 29781;

int remainingCycles = 0;

inline int elapsed()
{
    return TOTAL_CYCLES - remainingCycles;
}

/* Cycle emulation */
inline void tick()
{
    PPU::step();
    PPU::step();
    PPU::step();
    remainingCycles--;
}

/* Flags updating */
inline void upd_cv(uint8_t x, uint8_t y, int16_t r)
{
    P[C] = (r > 0xFF);
    P[V] = ~(x ^ y) & (x ^ r) & 0x80;
}

inline void upd_nz(uint8_t x)
{
    P[N] = x & 0x80;
    P[Z] = (x == 0);
}

// Does adding I to A cross a page?
inline bool cross(uint16_t a, uint8_t i)
{
    return ((a + i) & 0xFF00) != ((a & 0xFF00));
}

/* Memory access */
void dma_oam(uint8_t bank);
template <bool wr>
inline uint8_t access(uint16_t addr, uint8_t v = 0)
{
    uint8_t *r;
    switch (addr)
    {
    // RAM
    case 0x0000 ... 0x1FFF:
        r = &ram[addr % 0x800];
        if (wr)
            *r = v;
        return *r;
    // PPU
    case 0x2000 ... 0x3FFF:
        return PPU::access<wr>(addr % 8, v);
    // APU:
    case 0x4000 ... 0x4013:
    case 0x4015:
        return APU::access<wr>(elapsed(), addr, v);
    // Joypad 1
    case 0x4017:
        if (wr)
            return APU::access<wr>(elapsed(), addr, v);
        else
            return Joypad::read_state(1);
    // OAM DMA
    case 0x4014:
        if (wr)
            dma_oam(v);
        break;
    case 0x4016:
        // Joypad stroke
        if (wr)
        {
            Joypad::write_strobe(v & 1);
            break;
        }
        // Joypad 0
        else
            return Joypad::read_state(0);
    // Cartridge
    case 0x4018 ... 0xFFFF:
        return Cartridge::access<wr>(addr, v);
    }
    return 0;
}

inline uint8_t wr(uint16_t a, uint8_t v)
{
    tick();
    return access<1>(a, v);
}

inline uint8_t rd(uint16_t a)
{
    tick();
    return access<0>(a);
}

// Read from A and B and merge.
inline uint16_t rd16_d(uint16_t a, uint16_t b)
{
    return rd(a) | (rd(b) << 8);
}

inline uint16_t rd16(uint16_t a)
{
    return rd16_d(a, a + 1);
}

inline uint8_t push(uint8_t v)
{
    return wr(0x100 + (S--), v);
}

inline uint8_t pop()
{
    return rd(0x100 + (++S));
}

void dma_oam(uint8_t bank)
{
    for (int i = 0; i < 256; i++)
        wr(0x2014, rd(bank * 0x100 + i));
}

/* Addressing modes */
inline uint16_t imm()
{
    return PC++;
}

inline uint16_t imm16()
{
    PC += 2;
    return PC - 2;
}

inline uint16_t abs()
{
    return rd16(imm16());
}

// Exception
inline uint16_t _abx()
{
    tick();
    return abs() + X;
}

inline uint16_t abx()
{
    uint16_t a = abs();
    if (cross(a, X))
        tick();
    return a + X;
}

inline uint16_t aby()
{
    uint16_t a = abs();
    if (cross(a, Y))
        tick();
    return a + Y;
}

inline uint16_t zp()
{
    return rd(imm());
}

inline uint16_t zpx()
{
    tick();
    return (zp() + X) % 0x100;
}

inline uint16_t zpy()
{
    tick();
    return (zp() + Y) % 0x100;
}

inline uint16_t izx()
{
    uint8_t i = zpx();
    return rd16_d(i, (i + 1) % 0x100);
}

// Exception
inline uint16_t _izy()
{
    uint8_t i = zp();
    return rd16_d(i, (i + 1) % 0x100) + Y;
}

inline uint16_t izy()
{
    uint16_t a = _izy();
    if (cross(a - Y, Y))
        tick();
    return a;
}

/* STx */
template <uint8_t &r, Mode m>
void st()
{
    wr(m(), r);
}

// Exceptions
template <>
void st<A, izy>()
{
    tick();
    wr(_izy(), A);
}

template <>
void st<A, abx>()
{
    tick();
    wr(abs() + X, A);
}

template <>
void st<A, aby>()
{
    tick();
    wr(abs() + Y, A);
}

#define G             \
    uint16_t a = m(); \
    uint8_t p = rd(a) /* Fetch parameter */

// LDx
template <uint8_t &r, Mode m>
void ld()
{
    G;
    upd_nz(r = p);
}

// CMP, CPx
template <uint8_t &r, Mode m>
void cmp()
{
    G;
    upd_nz(r - p);
    P[C] = (r >= p);
}

/* Arithmetic and bitwise */
template <Mode m>
void ADC()
{
    G;
    int16_t r = A + p + P[C];
    upd_cv(A, p, r);
    upd_nz(A = r);
}

template <Mode m>
void SBC()
{
    G ^ 0xFF;
    int16_t r = A + p + P[C];
    upd_cv(A, p, r);
    upd_nz(A = r);
}

template <Mode m>
void nes_BIT()
{
    G;
    P[Z] = !(A & p);
    P[N] = p & 0x80;
    P[V] = p & 0x40;
}

template <Mode m>
void AND()
{
    G;
    upd_nz(A &= p);
}

template <Mode m>
void EOR()
{
    G;
    upd_nz(A ^= p);
}

template <Mode m>
void ORA()
{
    G;
    upd_nz(A |= p);
}

/* Read-Modify-Write */
template <Mode m>
void ASL()
{
    G;
    P[C] = p & 0x80;
    tick();
    upd_nz(wr(a, p << 1));
}

template <Mode m>
void LSR()
{
    G;
    P[C] = p & 0x01;
    tick();
    upd_nz(wr(a, p >> 1));
}

template <Mode m>
void ROL()
{
    G;
    uint8_t c = P[C];
    P[C] = p & 0x80;
    tick();
    upd_nz(wr(a, (p << 1) | c));
}

template <Mode m>
void ROR()
{
    G;
    uint8_t c = P[C] << 7;
    P[C] = p & 0x01;
    tick();
    upd_nz(wr(a, c | (p >> 1)));
}

template <Mode m>
void DEC()
{
    G;
    tick();
    upd_nz(wr(a, --p));
}

template <Mode m>
void INC()
{
    G;
    tick();
    upd_nz(wr(a, ++p));
}
#undef G

/* DEx, INx */
template <uint8_t &r>
void dec()
{
    upd_nz(--r);
    tick();
}

template <uint8_t &r>
void inc()
{
    upd_nz(++r);
    tick();
}

/* Bit shifting on the accumulator */
void ASL_A()
{
    P[C] = A & 0x80;
    upd_nz(A <<= 1);
    tick();
}

void LSR_A()
{
    P[C] = A & 0x01;
    upd_nz(A >>= 1);
    tick();
}

void ROL_A()
{
    uint8_t c = P[C];
    P[C] = A & 0x80;
    upd_nz(A = ((A << 1) | c));
    tick();
}

void ROR_A()
{
    uint8_t c = P[C] << 7;
    P[C] = A & 0x01;
    upd_nz(A = (c | (A >> 1)));
    tick();
}

/* Txx (move values between registers) */
template <uint8_t &s, uint8_t &d>
void tr()
{
    upd_nz(d = s);
    tick();
}

// TSX, exception.
template <>
void tr<X, S>()
{
    S = X;
    tick();
}

/* Stack operations */
void PLP()
{
    tick();
    tick();
    P.set(pop());
}

// B flag set
void PHP()
{
    tick();
    push(P.get() | (1 << 4));
}

void PLA()
{
    tick();
    tick();
    A = pop();
    upd_nz(A);
}

void PHA()
{
    tick();
    push(A);
}

/* Flow control (branches, jumps) */
template <Flag f, bool v>
void br()
{
    int8_t j = rd(imm());
    if (P[f] == v)
    {
        tick();
        PC += j;
    }
}

void JMP_IND()
{
    uint16_t i = rd16(imm16());
    PC = rd16_d(i, (i & 0xFF00) | ((i + 1) % 0x100));
}

void JMP()
{
    PC = rd16(imm16());
}

void JSR()
{
    uint16_t t = PC + 1;
    tick();
    push(t >> 8);
    push(t);
    PC = rd16(imm16());
}

/* Return instructions */
void RTS()
{
    tick();
    tick();
    PC = (pop() | (pop() << 8)) + 1;
    tick();
}

void RTI()
{
    PLP();
    PC = pop() | (pop() << 8);
}

// Clear and set flags.
template <Flag f, bool v>
void flag()
{
    P[f] = v;
    tick();
}

template <IntType t>
void INT()
{
    tick();
    // BRK already performed the fetch.
    if (t != BRK)
        tick();
    // Writes on stack are inhibited on RESET.
    if (t != RESET)
    {
        push(PC >> 8);
        push(PC & 0xFF);
        // Set B if BRK.
        push(P.get() | ((t == BRK) << 4));
    }
    else
    {
        S -= 3;
        tick();
        tick();
        tick();
    }
    P[I] = true;
    /*   NMI    Reset    IRQ     BRK  */
    constexpr uint16_t vect[] = {0xFFFA, 0xFFFC, 0xFFFE, 0xFFFE};
    PC = rd16(vect[t]);
    if (t == NMI)
        nmi = false;
}

void NOP()
{
    tick();
}

/* Execute a CPU instruction */
void exec()
{
    switch (rd(PC++)) // Fetch the opcode.
    {
    // Select the right function to emulate the instruction:
    case 0x00:
        return INT<BRK>();
    case 0x01:
        return ORA<izx>();
    case 0x05:
        return ORA<zp>();
    case 0x06:
        return ASL<zp>();
    case 0x08:
        return PHP();
    case 0x09:
        return ORA<imm>();
    case 0x0A:
        return ASL_A();
    case 0x0D:
        return ORA<abs>();
    case 0x0E:
        return ASL<abs>();
    case 0x10:
        return br<N, 0>();
    case 0x11:
        return ORA<izy>();
    case 0x15:
        return ORA<zpx>();
    case 0x16:
        return ASL<zpx>();
    case 0x18:
        return flag<C, 0>();
    case 0x19:
        return ORA<aby>();
    case 0x1D:
        return ORA<abx>();
    case 0x1E:
        return ASL<_abx>();
    case 0x20:
        return JSR();
    case 0x21:
        return AND<izx>();
    case 0x24:
        return nes_BIT<zp>();
    case 0x25:
        return AND<zp>();
    case 0x26:
        return ROL<zp>();
    case 0x28:
        return PLP();
    case 0x29:
        return AND<imm>();
    case 0x2A:
        return ROL_A();
    case 0x2C:
        return nes_BIT<abs>();
    case 0x2D:
        return AND<abs>();
    case 0x2E:
        return ROL<abs>();
    case 0x30:
        return br<N, 1>();
    case 0x31:
        return AND<izy>();
    case 0x35:
        return AND<zpx>();
    case 0x36:
        return ROL<zpx>();
    case 0x38:
        return flag<C, 1>();
    case 0x39:
        return AND<aby>();
    case 0x3D:
        return AND<abx>();
    case 0x3E:
        return ROL<_abx>();
    case 0x40:
        return RTI();
    case 0x41:
        return EOR<izx>();
    case 0x45:
        return EOR<zp>();
    case 0x46:
        return LSR<zp>();
    case 0x48:
        return PHA();
    case 0x49:
        return EOR<imm>();
    case 0x4A:
        return LSR_A();
    case 0x4C:
        return JMP();
    case 0x4D:
        return EOR<abs>();
    case 0x4E:
        return LSR<abs>();
    case 0x50:
        return br<V, 0>();
    case 0x51:
        return EOR<izy>();
    case 0x55:
        return EOR<zpx>();
    case 0x56:
        return LSR<zpx>();
    case 0x58:
        return flag<I, 0>();
    case 0x59:
        return EOR<aby>();
    case 0x5D:
        return EOR<abx>();
    case 0x5E:
        return LSR<_abx>();
    case 0x60:
        return RTS();
    case 0x61:
        return ADC<izx>();
    case 0x65:
        return ADC<zp>();
    case 0x66:
        return ROR<zp>();
    case 0x68:
        return PLA();
    case 0x69:
        return ADC<imm>();
    case 0x6A:
        return ROR_A();
    case 0x6C:
        return JMP_IND();
    case 0x6D:
        return ADC<abs>();
    case 0x6E:
        return ROR<abs>();
    case 0x70:
        return br<V, 1>();
    case 0x71:
        return ADC<izy>();
    case 0x75:
        return ADC<zpx>();
    case 0x76:
        return ROR<zpx>();
    case 0x78:
        return flag<I, 1>();
    case 0x79:
        return ADC<aby>();
    case 0x7D:
        return ADC<abx>();
    case 0x7E:
        return ROR<_abx>();
    case 0x81:
        return st<A, izx>();
    case 0x84:
        return st<Y, zp>();
    case 0x85:
        return st<A, zp>();
    case 0x86:
        return st<X, zp>();
    case 0x88:
        return dec<Y>();
    case 0x8A:
        return tr<X, A>();
    case 0x8C:
        return st<Y, abs>();
    case 0x8D:
        return st<A, abs>();
    case 0x8E:
        return st<X, abs>();
    case 0x90:
        return br<C, 0>();
    case 0x91:
        return st<A, izy>();
    case 0x94:
        return st<Y, zpx>();
    case 0x95:
        return st<A, zpx>();
    case 0x96:
        return st<X, zpy>();
    case 0x98:
        return tr<Y, A>();
    case 0x99:
        return st<A, aby>();
    case 0x9A:
        return tr<X, S>();
    case 0x9D:
        return st<A, abx>();
    case 0xA0:
        return ld<Y, imm>();
    case 0xA1:
        return ld<A, izx>();
    case 0xA2:
        return ld<X, imm>();
    case 0xA4:
        return ld<Y, zp>();
    case 0xA5:
        return ld<A, zp>();
    case 0xA6:
        return ld<X, zp>();
    case 0xA8:
        return tr<A, Y>();
    case 0xA9:
        return ld<A, imm>();
    case 0xAA:
        return tr<A, X>();
    case 0xAC:
        return ld<Y, abs>();
    case 0xAD:
        return ld<A, abs>();
    case 0xAE:
        return ld<X, abs>();
    case 0xB0:
        return br<C, 1>();
    case 0xB1:
        return ld<A, izy>();
    case 0xB4:
        return ld<Y, zpx>();
    case 0xB5:
        return ld<A, zpx>();
    case 0xB6:
        return ld<X, zpy>();
    case 0xB8:
        return flag<V, 0>();
    case 0xB9:
        return ld<A, aby>();
    case 0xBA:
        return tr<S, X>();
    case 0xBC:
        return ld<Y, abx>();
    case 0xBD:
        return ld<A, abx>();
    case 0xBE:
        return ld<X, aby>();
    case 0xC0:
        return cmp<Y, imm>();
    case 0xC1:
        return cmp<A, izx>();
    case 0xC4:
        return cmp<Y, zp>();
    case 0xC5:
        return cmp<A, zp>();
    case 0xC6:
        return DEC<zp>();
    case 0xC8:
        return inc<Y>();
    case 0xC9:
        return cmp<A, imm>();
    case 0xCA:
        return dec<X>();
    case 0xCC:
        return cmp<Y, abs>();
    case 0xCD:
        return cmp<A, abs>();
    case 0xCE:
        return DEC<abs>();
    case 0xD0:
        return br<Z, 0>();
    case 0xD1:
        return cmp<A, izy>();
    case 0xD5:
        return cmp<A, zpx>();
    case 0xD6:
        return DEC<zpx>();
    case 0xD8:
        return flag<D, 0>();
    case 0xD9:
        return cmp<A, aby>();
    case 0xDD:
        return cmp<A, abx>();
    case 0xDE:
        return DEC<_abx>();
    case 0xE0:
        return cmp<X, imm>();
    case 0xE1:
        return SBC<izx>();
    case 0xE4:
        return cmp<X, zp>();
    case 0xE5:
        return SBC<zp>();
    case 0xE6:
        return INC<zp>();
    case 0xE8:
        return inc<X>();
    case 0xE9:
        return SBC<imm>();
    case 0xEA:
        return NOP();
    case 0xEC:
        return cmp<X, abs>();
    case 0xED:
        return SBC<abs>();
    case 0xEE:
        return INC<abs>();
    case 0xF0:
        return br<Z, 1>();
    case 0xF1:
        return SBC<izy>();
    case 0xF5:
        return SBC<zpx>();
    case 0xF6:
        return INC<zpx>();
    case 0xF8:
        return flag<D, 1>();
    case 0xF9:
        return SBC<aby>();
    case 0xFD:
        return SBC<abx>();
    case 0xFE:
        return INC<_abx>();
    default:
    {
        std::cout << "Invalid OPcode! PC: " << PC << " OPcode: 0x" << std::hex << (int)rd(PC - 1) << "\n";
        return NOP();
    }
    }
}

void set_nmi(bool v)
{
    nmi = v;
}

void set_irq(bool v)
{
    irq = v;
}

int dmc_read(void *, cpu_addr_t addr)
{
    return access<0>(addr);
}

/* Turn on the CPU */
void power()
{
    remainingCycles = 0;
    P.set(0x04);
    A = X = Y = S = 0x00;
    memset(ram, 0xFF, sizeof(ram));

    nmi = irq = false;
    INT<RESET>();
}

/* Run the CPU for roughly a frame */
void run_frame()
{
    remainingCycles += TOTAL_CYCLES;
    while (remainingCycles > 0)
    {
        if (nmi)
            INT<NMI>();
        else if (irq and !P[I])
            INT<IRQ>();
        exec();
    }
    APU::run_frame(elapsed());
}

} // namespace CPU
