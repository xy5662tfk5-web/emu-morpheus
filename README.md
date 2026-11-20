# E-MU Morpheus — research emulator

An open research project aimed at an **authentic software emulation** of the E-MU Morpheus, with an Intel macOS audio-plugin target as a long-term deliverable.

> **Status: research scaffold only.** This repository does not yet emulate the Morpheus, produce Morpheus audio, or contain E-MU firmware/sound ROMs. A working host/plugin shell is not evidence of an authentic sound engine.

## Target

- macOS Intel (x86_64), initially macOS Monterey compatibility
- AU for Logic Pro; VST3 and Standalone are later targets
- Original Morpheus firmware and sound ROM support, only when legally supplied by the user
- Reverse-engineered CPU, memory map, proprietary E-MU sound ASICs, and Z-Plane behavior

## Current engineering stages

1. Establish a portable C++ research core and automated tests.
2. Document the hardware and firmware/ROM formats from lawful sources.
3. Integrate and validate a Motorola 68020 CPU core.
4. Research the Morpheus-specific ASICs and audio data path.
5. Implement and validate Z-Plane filter structures and modulation.
6. Add the JUCE audio-plugin host layer after the sound engine has a testable API.

## Important distinction

The Morpheus is not simply a 68020 plus an ADSP-2115. The proprietary E-MU sound-generation hardware and its control/data path are essential. A generic biquad morphing filter or a Proteus VX sample player is **not** an authentic Morpheus implementation.

## Build the current scaffold

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

The current tests verify only the research-core scaffolding and must not be interpreted as hardware-emulation tests.

## Third-party components

Potential CPU-core research candidate: [Moira](https://github.com/dirkwhoffmann/Moira), an independently maintained Motorola 68k emulator under the MIT license. It has not yet been vendored or integrated; its 68020 configuration, API, timing behavior, and licensing notices must be reviewed first.

No proprietary E-MU ROM, firmware, factory preset bank, or service data is included.
