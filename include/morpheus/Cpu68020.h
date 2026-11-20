#pragma once

#include "morpheus/MorpheusCore.h"
#include <Moira.h>

namespace morpheus {

// Adapter between Moira's Motorola 68020 implementation and the machine bus.
// The Morpheus address map is deliberately not guessed here: the caller owns
// the bus implementation and can later provide the verified hardware map.
class Cpu68020 final : public moira::Moira {
public:
    explicit Cpu68020(CpuBus& bus);

    void hardReset();
    void stepInstruction();

    [[nodiscard]] std::uint32_t programCounter() const noexcept;

protected:
    void sync(int cycles) override;
    moira::u8 read8(moira::u32 address) const override;
    moira::u16 read16(moira::u32 address) const override;
    moira::u32 read32(moira::u32 address) const override;
    void write8(moira::u32 address, moira::u8 value) const override;
    void write16(moira::u32 address, moira::u16 value) const override;
    void write32(moira::u32 address, moira::u32 value) const override;

private:
    CpuBus& bus_;
};

} // namespace morpheus
