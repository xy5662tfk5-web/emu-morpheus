#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

namespace morpheus {

// This is deliberately an interface boundary, not an implementation of the
// proprietary E-MU hardware. No memory map or ASIC behavior is assumed here.
enum class CpuModel : std::uint8_t { Motorola68020 };

struct RomImageView {
    std::span<const std::byte> bytes{};
    std::string_view label{};
};

class CpuBus {
public:
    virtual ~CpuBus() = default;
    virtual std::uint8_t read8(std::uint32_t address) = 0;
    virtual void write8(std::uint32_t address, std::uint8_t value) = 0;
};

class SoundEngine {
public:
    virtual ~SoundEngine() = default;
    virtual void reset() = 0;
    virtual void render(float* left, float* right, std::size_t frames) = 0;
};

class MorpheusCore final {
public:
    static constexpr std::string_view projectName = "E-MU Morpheus research core";

    void reset() noexcept;
    [[nodiscard]] bool isInitialized() const noexcept;

private:
    bool initialized_{false};
};

} // namespace morpheus
