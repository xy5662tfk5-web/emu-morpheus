# E-MU Morpheus — research emulator

An open research project aimed at an **authentic software emulation** of the E-MU Morpheus, with an Intel macOS audio-plugin target as a long-term deliverable.

> **Status: early research scaffold.** The Motorola 68020 CPU core is integrated and has passed a reset-vector/NOP smoke test in Intel macOS CI. The project does not yet boot Morpheus firmware, emulate its proprietary sound ASICs, or produce Morpheus audio.

## Target

- macOS Intel (x86_64), initially macOS Monterey compatibility
- AU for Logic Pro; VST3 and Standalone are later targets
- Original Morpheus firmware and sound ROM support, only when legally supplied by the user
- Reverse-engineered CPU, memory map, proprietary E-MU sound ASICs, and Z-Plane behavior

## Current engineering stages

1. Establish a portable C++ research core and automated tests.
2. Document the hardware and firmware/ROM formats from lawful sources.
3. Integrate and validate a Motorola 68020 CPU core. **Initial integration complete; only a minimal smoke test exists.**
4. Research the Morpheus-specific ASICs, memory map, and audio data path.
5. Implement and validate the sample playback, voice architecture, and Z-Plane processing.
6. Add the JUCE audio-plugin host layer after the sound engine has a testable API.

## Important distinction

The Morpheus is not simply a 68020 plus an ADSP-2115. Contemporary hardware reporting identifies five custom AMI ASICs on the main board; the ADSP-2115 on a small daughterboard is reported to produce the digital effects. The proprietary E-MU sound-generation hardware and its control/data path are essential. A generic biquad morphing filter or a Proteus VX sample player is **not** an authentic Morpheus implementation.

## Build the current scaffold

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

The current tests verify the CPU adapter's reset-vector/NOP path and scaffold interfaces only. They are not firmware-boot, sound-engine, or hardware-conformance tests.

## Third-party components

- [Moira](https://github.com/dirkwhoffmann/Moira): integrated from upstream commit `ce28239b0507ebfe1bf2ad2b9c9252dc2f2cc031`, configured for Motorola 68020. Moira is MIT-licensed; see its upstream license.
- [Hardware research notes](docs/HARDWARE_RESEARCH.md): documented facts, source links, and unresolved reverse-engineering questions.

No proprietary E-MU ROM, firmware, factory preset bank, or service data is included.
