#include <alu.hpp>

ALU::ALU(MMU* mmu_ptr): mmu(mmu_ptr){}

void ALU::update_flags()
{
    if((TEMP1 & D7) == D7) SF |= HX_SIGN;
    else SF &= ~HX_SIGN;
    if(TEMP1 == 0x00) SF |= HX_ZERO;
    else SF &= ~HX_ZERO;
}

void ALU::update_flags_on_ld(REG r)
{
    TEMP1 = mmu->tapREG(r);
    SF = mmu->tapREG(ST);
    update_flags();
    mmu->ld(ST, SF);
}

void ALU::adc(ADR mode, u16 addr, u8 off)
{
    if(mode == -1) fetchIMD(off);
    else fetchMEM(mode, addr, off);
    TEMP1 = mmu->tapREG(A);   
    u16 TEMP = TEMP1 + TEMP2 + ((SF & HX_CARY) == HX_CARY ? 1 : 0);
    SF &= ~(HX_CARY | HX_OVFW);
    if(TEMP > 0xFF) SF |= HX_CARY;
    if((TEMP ^ TEMP1) & (TEMP ^ TEMP2) & D7) SF |= HX_OVFW; 
    TEMP1 += TEMP2;
    loadREG(A);
}

void ALU::sbc(ADR mode, u16 addr, u8 off)
{
    if(mode == -1) fetchIMD(off);
    else fetchMEM(mode, addr, off);
    TEMP1 = mmu->tapREG(A);
    u16 TEMP = TEMP1 - TEMP2 + ((SF & HX_CARY) == HX_CARY ? 1 : 0);
    SF &= ~(HX_CARY | HX_OVFW);
    if(!(TEMP < 0x00)) SF |= HX_CARY;
    if((TEMP ^ TEMP1) & (TEMP ^ ~TEMP2) & D7) SF |= HX_OVFW;
    TEMP1 -= TEMP2;
    loadREG(A);
}

void ALU::ana(ADR mode, u16 addr, u8 off)
{
    if(mode == -1) fetchIMD(off);
    else fetchMEM(mode, addr, off);
    TEMP1 = mmu->tapREG(A);
    TEMP1 &= TEMP2;
    loadREG(A); 
}

void ALU::eor(ADR mode, u16 addr, u8 off)
{
    if(mode == -1) fetchIMD(off);
    else fetchMEM(mode, addr, off);
    TEMP1 = mmu->tapREG(A);
    TEMP1 ^= TEMP2;
    loadREG(A);
}

void ALU::ora(ADR mode, u16 addr, u8 off)
{
    if(mode == -1) fetchIMD(off);
    else fetchMEM(mode, addr, off);
    TEMP1 = mmu->tapREG(A);
    TEMP1 |= TEMP2;
    loadREG(A);
}

void ALU::cmp(REG r, ADR mode, u16 addr, u8 off)
{
    if(mode == -1) fetchIMD(off);
    else fetchMEM(mode, addr, off);
    SF &= ~(HX_CARY);
    TEMP1 = mmu->tapREG(r);
    if(TEMP1 >= TEMP2) { SF |= HX_CARY; /* A >= M => C = 1 */ }
    if(TEMP1 == TEMP2) { SF |= HX_ZERO; /* A == M => Z = 1 */ }
    mmu->ld(ST, SF);
}

void ALU::asl(ADR mode, u16 addr, u8 off)
{
    if(mode == -1) fetchREG(A);
    else fetchMEM(mode, addr, off);
    TEMP1 = TEMP2 << 1;
    SF |= (TEMP2 & D7) >> 7;
    if(mode == -1) loadREG(A);
    else loadMEM(mode, addr, off);
}

void ALU::lsr(ADR mode, u16 addr, u8 off)
{
    if(mode == -1) fetchREG(A);
    else fetchMEM(mode, addr, off);
    TEMP1 = TEMP2 >> 1;
    SF |= (TEMP2 & D0);
    if(mode == -1) loadREG(A);
    else loadMEM(mode, addr, off);
}

void ALU::rol(ADR mode, u16 addr, u8 off)
{
    if(mode == -1) fetchREG(A);
    else fetchMEM(mode, addr, off);
    TEMP1 = TEMP2 << 1;
    TEMP1 |= (SF & HX_CARY);
    SF |= (TEMP2 & D7) >> 7;
    if(mode == -1) loadREG(A);
    else loadMEM(mode, addr, off);
}

void ALU::ror(ADR mode, u16 addr, u8 off)
{
    if(mode == -1) fetchREG(A);
    else fetchMEM(mode, addr, off);
    TEMP1 = TEMP2 >> 1;
    TEMP1 |= (SF & HX_CARY) << 7;
    SF |= (TEMP2 & D0);
    if(mode == -1) loadREG(A);
    else loadMEM(mode, addr, off);
}

void ALU::inc(REG r)
{
    fetchREG(r);
    TEMP1 = TEMP2;
    TEMP1 += 0x01;
    loadREG(r);
}

void ALU::dec(REG r)
{
    fetchREG(r);
    TEMP1 = TEMP2;
    TEMP1 -= 0x01;
    loadREG(r);
}

void ALU::inc(ADR mode, u16 addr, u8 off)
{
    fetchMEM(mode, addr, off);
    TEMP1 = TEMP2;
    TEMP1 += 0x01;
    loadMEM(mode, addr, off);
}

void ALU::dec(ADR mode, u16 addr, u8 off)
{
    fetchMEM(mode, addr, off);
    TEMP1 = TEMP2;
    TEMP1 -= 0x01;
    loadMEM(mode, addr, off);
}

void ALU::set_flag(u8 mask)
{
    SF = mmu->tapREG(ST);
    SF |= mask;
    mmu->ld(ST, SF);
}

void ALU::clr_flag(u8 mask)
{
    SF = mmu->tapREG(ST);
    SF &= ~mask;
    mmu->ld(ST, SF);
}

void ALU::fetchIMD(u8 off)
{
    TEMP2 = off;
    SF = mmu->tapREG(ST);
}

void ALU::fetchMEM(ADR mode, u16 addr, u8 off)
{
    TEMP2 = mmu->fetch_mem(mmu->get_addr(mode, addr, off));
    SF = mmu->tapREG(ST);
}

void ALU::fetchREG(REG r)
{
    TEMP2 = mmu->tapREG(r);
    SF = mmu->tapREG(ST);
}

void ALU::loadMEM(ADR mode, u16 addr, u8 off)
{
    update_flags();
    mmu->load_mem(mmu->get_addr(mode, addr, off), TEMP1);
    mmu->ld(ST, SF);
}

void ALU::loadREG(REG r)
{
    update_flags();
    mmu->ld(r, TEMP1);
    mmu->ld(ST, SF);
}
