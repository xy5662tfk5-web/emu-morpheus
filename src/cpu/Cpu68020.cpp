#include "morpheus/Cpu68020.h"

namespace morpheus {

Cpu68020::Cpu68020(CpuBus& bus) : bus_(bus) {
    setModel(moira::Model::M68020);
}

void Cpu68020::hardReset() {
    reset();
}

void Cpu68020::stepInstruction() {
    execute();
}

std::uint32_t Cpu68020::programCounter() const noexcept {
    return getPC();
}

void Cpu68020::sync(int cycles) {
    moira::Moira::sync(cycles);
}

moira::u8 Cpu68020::read8(moira::u32 address) const {
    return bus_.read8(address);
}

moira::u16 Cpu68020::read16(moira::u32 address) const {
    const auto hi = static_cast<moira::u16>(read8(address));
    const auto lo = static_cast<moira::u16>(read8(address + 1));
    return static_cast<moira::u16>((hi << 8) | lo);
}

moira::u32 Cpu68020::read32(moira::u32 address) const {
    const auto hi = static_cast<moira::u32>(read16(address));
    const auto lo = static_cast<moira::u32>(read16(address + 2));
    return (hi << 16) | lo;
}

void Cpu68020::write8(moira::u32 address, moira::u8 value) const {
    bus_.write8(address, value);
}

void Cpu68020::write16(moira::u32 address, moira::u16 value) const {
    write8(address, static_cast<moira::u8>(value >> 8));
    write8(address + 1, static_cast<moira::u8>(value));
}

void Cpu68020::write32(moira::u32 address, moira::u32 value) const {
    write16(address, static_cast<moira::u16>(value >> 16));
    write16(address + 2, static_cast<moira::u16>(value));
}

} // namespace morpheus
