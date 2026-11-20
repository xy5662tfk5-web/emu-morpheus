#include "morpheus/SysExPresetBank.h"

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

namespace {

std::vector<std::uint8_t> makePreset(std::uint16_t number,
                                     const std::string& name) {
    std::vector<std::uint8_t> message(566, 0);
    message[0] = 0xF0;
    message[1] = 0x18;
    message[2] = 0x0C;
    message[3] = 7;
    message[4] = 0x01;
    message[5] = 0x01;
    message[6] = static_cast<std::uint8_t>(number & 0x7F);
    message[7] = static_cast<std::uint8_t>((number >> 7) & 0x7F);

    for (std::size_t i = 0; i < 12; ++i) {
        const auto ch = i < name.size() ? static_cast<std::uint8_t>(name[i]) : 32;
        message[8 + i * 2] = ch;
        // A non-zero upper seven-bit part is valid for a name word.
        message[9 + i * 2] = (i == 0) ? 1 : 0;
    }

    unsigned int sum = 0;
    for (std::size_t i = 8; i < 564; ++i)
        sum += message[i];
    message[564] = static_cast<std::uint8_t>(sum & 0x7F);
    message[565] = 0xF7;
    return message;
}

void append(std::vector<std::uint8_t>& into,
            const std::vector<std::uint8_t>& message) {
    into.insert(into.end(), message.begin(), message.end());
}

} // namespace

int main() {
    std::vector<std::uint8_t> bank;
    append(bank, makePreset(0, "Stereo Piano"));
    bank.push_back(0x00);
    append(bank, makePreset(0x123, "Warm Bass"));

    auto parsed = morpheus::parsePresetBankSysEx(bank.data(), bank.size());
    assert(parsed.presets.size() == 2);
    assert(parsed.presets[0].number == 0);
    assert(parsed.presets[0].name == "Stereo Piano");
    assert(parsed.presets[1].number == 0x123);
    assert(parsed.presets[1].name == "Warm Bass");
    assert(parsed.invalidMessages == 0);

    auto damaged = makePreset(3, "Damaged");
    damaged[200] ^= 1;
    auto bad = morpheus::parsePresetBankSysEx(damaged.data(), damaged.size());
    assert(bad.presets.empty());
    assert(bad.invalidMessages == 1);

    auto foreign = makePreset(4, "Proteus");
    foreign[2] = 0x04;
    auto other = morpheus::parsePresetBankSysEx(foreign.data(), foreign.size());
    assert(other.presets.empty());
    assert(other.ignoredMessages == 1);

    std::vector<std::uint8_t> hyper(484, 0);
    hyper[0] = 0xF0;
    hyper[1] = 0x18;
    hyper[2] = 0x0C;
    hyper[3] = 7;
    hyper[4] = 0x45;
    hyper.back() = 0xF7;
    auto nonPreset = morpheus::parsePresetBankSysEx(hyper.data(), hyper.size());
    assert(nonPreset.presets.empty());
    assert(nonPreset.ignoredMessages == 1);

    const std::uint8_t truncated[] = {0xF0, 0x18, 0x0C, 7, 1};
    auto incomplete = morpheus::parsePresetBankSysEx(truncated, sizeof(truncated));
    assert(incomplete.invalidMessages == 1);
    return 0;
}
