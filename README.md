# E-MU Morpheus — research emulator

An open research project aimed at an **authentic software emulation** of the E-MU Morpheus.

> **Status: early research scaffold + AU host shell.** The Motorola 68020 CPU core is integrated and has passed a reset-vector/NOP smoke test. An initial Intel macOS Audio Unit target is being added. It currently produces only a temporary MIDI-triggered sine test tone; it does not boot Morpheus firmware or reproduce Morpheus sounds.

## Target

- macOS Intel (x86_64), initially macOS Monterey compatibility
- AU for Logic Pro
- Original Morpheus firmware and sound ROM support, only when lawfully supplied
- Reverse-engineered CPU, memory map, proprietary E-MU sound ASICs, and Z-Plane behavior

## Current engineering stages

1. Portable C++ research core and automated tests.
2. Hardware and firmware/ROM format research from lawful sources.
3. Motorola 68020 CPU integration. **Initial integration complete; only a minimal smoke test exists.**
4. JUCE-based AU host shell. **Initial MIDI test-tone shell; requires CI validation and DAW testing.**
5. Research and implement the Morpheus-specific ASICs, memory map, and audio data path.
6. Implement and validate sample playback, voice architecture, and authentic Z-Plane processing.

## Important distinction

The Morpheus is not simply a 68020 plus an ADSP-2115. Contemporary hardware reporting identifies five custom AMI ASICs on the main board; the ADSP-2115 on a small daughterboard is reported to produce the digital effects. The proprietary E-MU sound-generation hardware and its control/data path are essential. A generic biquad morphing filter or a Proteus VX sample player is **not** an authentic Morpheus implementation.

## Build

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

On macOS, CMake fetches JUCE 8.0.6 and builds the AU target. GitHub Actions uploads the Intel AU component as a workflow artifact when the build succeeds. The component is an integration/test shell, not a finished instrument.

## Third-party components

- [Moira](https://github.com/dirkwhoffmann/Moira): integrated from upstream commit `ce28239b0507ebfe1bf2ad2b9c9252dc2f2cc031`, configured for Motorola 68020. MIT-licensed; see upstream license.
- [JUCE](https://github.com/juce-framework/JUCE): pinned to 8.0.6 for the AU host layer. JUCE is distributed under its applicable open-source/commercial licensing terms; see the upstream JUCE license.
- [Hardware research notes](docs/HARDWARE_RESEARCH.md): documented facts, source links, and unresolved reverse-engineering questions.

No proprietary E-MU ROM, firmware, factory preset bank, or service data is included.
