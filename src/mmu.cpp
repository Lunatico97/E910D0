#include <mmu.hpp>

MMU::MMU(CardROM* cptr, PPU* pptr)
{
    reset();
    load_reg(SP, 0xFF);
    crom = cptr;
    ppu = pptr;
}

MMU::~MMU() {}

void MMU::push(REG r)
{
    if(fetch_reg(SP) > 0x00)
    {
        store(SP_INDEX | fetch_reg(SP), fetch_reg(r));
        load_reg(SP, fetch_reg(SP)-0x01);
    }
    else throw(std::runtime_error("Stack overflow !"));
}

void MMU::pop(REG r)
{
    if(fetch_reg(SP) <= 0xFF)
    {
        load_reg(SP, fetch_reg(SP)+0x01);
        load_reg(r, retreive(SP_INDEX | fetch_reg(SP)));
    }
    else throw(std::runtime_error("Stack underflow !"));
}

void MMU::tr(REG des, REG src)
{
    load_reg(des, fetch_reg(src));
    if(des != SP) updf(des);
}

void MMU::ld(REG des, u8 value)
{
    load_reg(des, value);
    updf(des);
}

void MMU::ld(REG des, REG off, u16 addr)
{
    if(off != REG::NON) load_reg(des, retreive(addr + fetch_reg(off)));
    else load_reg(des, retreive(addr));
    updf(des);
}

void MMU::st(REG src, REG off, u16 addr)
{
    if(off != REG::NON) store(addr + fetch_reg(off), fetch_reg(src));
    else store(addr, fetch_reg(src));
}

void MMU::ldo(REG des, REG off, u16 addr)
{
    if(off != REG::NON) load_reg(des, retreive((addr & 0x00FF) + fetch_reg(off)));
    else load_reg(des, retreive(addr & 0x00FF));
    updf(des);
}

void MMU::sto(REG src, REG off, u16 addr)
{
    if(off != REG::NON) store((addr & 0x00FF) + fetch_reg(off), fetch_reg(src));
    else store((addr & 0x00FF), fetch_reg(src));
}

void MMU::ldi(REG des, REG off, u16 addr)
{
    u16 act_addr = get_addr(ADR::IXI, addr, fetch_reg(off));
    load_reg(des, retreive(act_addr));
    updf(des);
}

void MMU::sti(REG src, REG off, u16 addr)
{
    u16 act_addr = get_addr(ADR::IXI, addr, fetch_reg(off));
    store(act_addr, fetch_reg(src));
}

void MMU::ldix(REG des, REG off, u16 addr)
{
    u16 base_addr = get_addr(ADR::IND, addr, 0x00);
    load_reg(des, retreive(base_addr + fetch_reg(off)));
    updf(des);
}

void MMU::stix(REG src, REG off, u16 addr)
{
    u16 base_addr = get_addr(ADR::IND, addr, 0x00);
    store(base_addr + fetch_reg(off), fetch_reg(src));
}

u16 MMU::get_addr(ADR mode, u16 addr, u8 off)
{
    u16 res_addr;

    switch(mode)
    {
        case ADR::ABS: res_addr = addr; break;
        case ADR::ABX: res_addr = addr + fetch_reg(X); break;
        case ADR::ABY: res_addr = addr + fetch_reg(Y); break;
        case ADR::IND: res_addr = retreive(addr) | ((retreive(addr+1) & 0x00FF) << 8); break;
        case ADR::IIX: res_addr = (retreive(addr) | ((retreive(addr+1) & 0x00FF) << 8)) + static_cast<u16>(off); break;
        case ADR::IXI: res_addr = retreive(addr+off) | ((retreive(addr+off+1) & 0x00FF) << 8) ; break;
        case ADR::REL: if((off & D7) == D7) res_addr = fetch_pc() - ((~off+1) & 0x00FF);
                       else res_addr = fetch_pc() + off; 
                       break;
        case ADR::ZER: res_addr = static_cast<u16>(off); break;
        case ADR::ZEX: res_addr = static_cast<u16>(fetch_reg(X) + off); break;
        case ADR::ZEY: res_addr = static_cast<u16>(fetch_reg(Y) + off); break;
        default: break;
    }

    return res_addr;
}

void MMU::updf(REG r)
{
    u8 flags = fetch_reg(ST);
    u8 value = fetch_reg(r);
    if((value & D7) == D7) flags |= D7;
    else flags &= ~D7;
    if(value == 0x00) flags |= D1;
    else flags &= ~D1;
    load_reg(ST, flags);
}

void MMU::reset()
{
    for(int i = 0; i < 2048; ++i) RAM[i] = 0x00;
    for(int i=0; i<7; i++) BANK[i] = 0x0000;
}

u8 MMU::fetch_reg(REG r)
{
    return *(BANK+r);
}

u16 MMU::fetch_pc()
{
    return (static_cast<u16>(*(BANK+PCH)) << 8) | static_cast<u16>(*(BANK+PCL));
}

void MMU::load_reg(REG r, const u8& value)
{
    *(BANK+r) = value;
}

void MMU::load_pc(const u16& addr)
{
    *(BANK+PCL) = (addr & 0x00FF);
    *(BANK+PCH) = (addr & 0xFF00) >> 8;
}

void MMU::store(u16 m_address, u8 value)
{
    if(m_address >= 0x0000 && m_address < 0x2000) RAM[(m_address & 0x07FF)] = value;
    else if(m_address >= 0x2000 && m_address < 0x4000) ppu->write_from_cpu((m_address & 0x2007), value);
    else if(m_address >= 0x4000 && m_address < 0x4020) return;
    else if(m_address >= 0x4020 && m_address < 0x8000) return;
    else return;
    // else crom->write_from_cpu(); 
    // Right now, we use only mapper 000 that doesn't perform writes on card
}

u8 MMU::retreive(u16 m_address)
{
    if(m_address >= 0x0000 && m_address < 0x2000) return RAM[(m_address & 0x07FF)];
    else if(m_address >= 0x2000 && m_address < 0x4000) return ppu->read_from_cpu(m_address & 0x2007);
    else if(m_address >= 0x4000 && m_address < 0x4020) return 0x00;
    else if(m_address >= 0x4020 && m_address < 0x8000) return 0x00;
    else return crom->read_from_cpu(m_address);
}
