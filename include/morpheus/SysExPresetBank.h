#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace morpheus {

struct PresetBankEntry {
    std::uint16_t number = 0;
    std::string name;
};

struct PresetBankParseResult {
    std::vector<PresetBankEntry> presets;
    std::size_t ignoredMessages = 0;
    std::size_t invalidMessages = 0;
};

/// Parse concatenated E-MU Morpheus preset-data SysEx messages.
/// This reads preset metadata only; it does not load samples or emulate audio.
PresetBankParseResult parsePresetBankSysEx(const std::uint8_t* data,
                                           std::size_t size);

} // namespace morpheus
