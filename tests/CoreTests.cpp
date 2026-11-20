#include "morpheus/Cpu68020.h"
#include "morpheus/MorpheusCore.h"

#include <array>
#include <cassert>
#include <cstdint>

namespace {

class TestBus final : public morpheus::CpuBus {
public:
    static constexpr std::uint32_t memorySize = 1U << 20;
    std::array<std::uint8_t, memorySize> memory{};

    std::uint8_t read8(std::uint32_t address) override {
        assert(address < memory.size());
        return memory[address];
    }

    void write8(std::uint32_t address, std::uint8_t value) override {
        assert(address < memory.size());
        memory[address] = value;
    }

    void write16(std::uint32_t address, std::uint16_t value) {
        write8(address, static_cast<std::uint8_t>(value >> 8));
        write8(address + 1, static_cast<std::uint8_t>(value));
    }

    void write32(std::uint32_t address, std::uint32_t value) {
        write16(address, static_cast<std::uint16_t>(value >> 16));
        write16(address + 2, static_cast<std::uint16_t>(value));
    }
};

} // namespace

int main() {
    morpheus::MorpheusCore core;
    core.reset();
    assert(!core.isInitialized());

    // Minimal 68020 reset-vector + NOP smoke test. This validates the CPU
    // adapter and big-endian bus bridge, not the Morpheus firmware or ASICs.
    TestBus bus;
    bus.write32(0x00000000, 0x0000F000); // Initial supervisor stack pointer
    bus.write32(0x00000004, 0x00000100); // Reset program counter
    bus.write16(0x00000100, 0x4E71);      // NOP

    morpheus::Cpu68020 cpu(bus);
    cpu.hardReset();
    assert(cpu.programCounter() == 0x00000100);
    cpu.stepInstruction();
    assert(cpu.programCounter() == 0x00000102);

    return 0;
}
