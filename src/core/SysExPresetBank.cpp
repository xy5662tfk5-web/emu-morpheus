#include "morpheus/SysExPresetBank.h"

#include <string>

namespace morpheus {
namespace {

constexpr std::uint8_t kSysExStart = 0xF0;
constexpr std::uint8_t kSysExEnd = 0xF7;
constexpr std::uint8_t kEmuManufacturer = 0x18;
constexpr std::uint8_t kMorpheusProduct = 0x0C;
constexpr std::uint8_t kPresetDataCommand = 0x01;
constexpr std::uint8_t kPresetDataVersion = 0x01;
constexpr std::size_t kPresetMessageBytes = 566;
constexpr std::size_t kDataOffset = 8;
constexpr std::size_t kChecksumOffset = kPresetMessageBytes - 2;
constexpr std::size_t kNameWords = 12;

bool isMorpheusPreset(const std::uint8_t* message, std::size_t length) {
    return length == kPresetMessageBytes
        && message[0] == kSysExStart
        && message[1] == kEmuManufacturer
        && message[2] == kMorpheusProduct
        && message[3] <= 0x0F
        && message[4] == kPresetDataCommand
        && message[5] == kPresetDataVersion
        && message[length - 1] == kSysExEnd;
}

bool hasValidDataAndChecksum(const std::uint8_t* message) {
    unsigned int sum = 0;
    for (std::size_t i = kDataOffset; i < kChecksumOffset; ++i) {
        if (message[i] > 0x7F)
            return false;
        sum += message[i];
    }
    return message[kChecksumOffset] == static_cast<std::uint8_t>(sum & 0x7F);
}

std::string decodePresetName(const std::uint8_t* message) {
    std::string name;
    name.reserve(kNameWords);
    for (std::size_t i = 0; i < kNameWords; ++i) {
        // Each character is the low seven bits of a fourteen-bit word.
        // The high seven bits are not required to be zero.
        const auto character = message[kDataOffset + i * 2] & 0x7F;
        name.push_back(character >= 32 && character < 127
                           ? static_cast<char>(character)
                           : '?');
    }
    while (!name.empty() && name.back() == ' ')
        name.pop_back();
    return name;
}

} // namespace

PresetBankParseResult parsePresetBankSysEx(const std::uint8_t* data,
                                           std::size_t size) {
    PresetBankParseResult result;
    if (data == nullptr || size == 0)
        return result;

    std::size_t cursor = 0;
    while (cursor < size) {
        while (cursor < size && data[cursor] != kSysExStart)
            ++cursor;
        if (cursor >= size)
            break;

        const auto start = cursor++;
        while (cursor < size && data[cursor] != kSysExEnd)
            ++cursor;
        if (cursor >= size) {
            ++result.invalidMessages;
            break;
        }

        const auto length = cursor - start + 1;
        const auto* message = data + start;
        ++cursor;

        if (!isMorpheusPreset(message, length)) {
            ++result.ignoredMessages;
            continue;
        }
        if (!hasValidDataAndChecksum(message)) {
            ++result.invalidMessages;
            continue;
        }

        const auto number = static_cast<std::uint16_t>(
            message[6] | (static_cast<std::uint16_t>(message[7]) << 7));
        result.presets.push_back({number, decodePresetName(message)});
    }
    return result;
}

} // namespace morpheus
