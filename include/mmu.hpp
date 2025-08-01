#include <registers.hpp>
#include <memory.hpp>

#ifndef _MMU_H_
#define _MMU_H_

#define SP_INDEX 0x0100

enum ADR
{
    ABS = 0, ABX, ABY, IND, IIX, IXI, REL, ZER, ZEX, ZEY
};

class MMU
{
    public:
        MMU(CardROM *cptr, PPU* pptr);
        ~MMU();

        // Memory interactors
        void load_mem(u16 address, u8 value);
        u8 fetch_mem(u16 address);

        // Program Counter
        void init_pc(u16 value);

        // Stack Pointer
        void push(REG r);
        void pop(REG r);

        // Implied Transfers
        void tr(REG des, REG src);

        // Immediate Transfers
        void ld(REG des, u8 value);

        // Addressed Transfers
        void ld(REG des, REG off, u16 addr);
        void st(REG des, REG off, u16 addr);

        // Zero-Addressed Transfers
        void ldo(REG des, REG off, u16 addr);
        void sto(REG des, REG off, u16 addr);

        // Indexed Indirect Transfers
        void ldi(REG des, REG off, u16 addr);
        void sti(REG des, REG off, u16 addr);

        // Indirect Indexed Transfers
        void ldix(REG des, REG off, u16 addr);
        void stix(REG des, REG off, u16 addr);

        // Addressing
        u16 get_addr(ADR mode, u16 addr, u8 off);

        // Taps
        u8 tapREG(REG r);
        u16 tapPC();

    private:
        Registers rb;
        Memory* mem;
};

#endif