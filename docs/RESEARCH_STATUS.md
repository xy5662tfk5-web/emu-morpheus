# Research status

## Implemented

- C++20 project scaffold and Intel macOS CI.
- Pinned MIT-licensed Moira Motorola 68k core, configured for the 68020.
- A bus adapter that translates Moira's 8/16/32-bit big-endian accesses to the project's byte bus.
- A small reset-vector/NOP smoke test for the CPU adapter.

## Confirmed project direction

- The goal is a real Morpheus emulation, not a sound-alike plugin.
- The Motorola 68020 is a candidate CPU subsystem to emulate.
- Morpheus-specific sound-generation ASICs and Z-Plane processing remain major reverse-engineering tasks.
- The ADSP-2115 is associated with the digital-effects board; its exact role and integration must be established from board-level evidence before implementation.

## Not yet established

- A complete, authoritative Morpheus memory map.
- Exact CPU firmware boot requirements and peripheral register behavior.
- Proprietary sound ASIC interfaces, microcode, and signal path.
- A validated model of the Morpheus Z-Plane filter algorithms.
- A lawful, complete firmware and sound-ROM set for test fixtures.

## CPU dependency

Moira is included from upstream commit `ce28239b0507ebfe1bf2ad2b9c9252dc2f2cc031`, under its MIT license. Its 68020 model is selected explicitly. The adapter currently delegates to an abstract byte-addressed bus; it does not claim that the Morpheus hardware memory map is known.

## Test policy

Tests must distinguish:
1. scaffold/interface tests;
2. CPU instruction and bus tests;
3. firmware boot tests;
4. sound-engine and Z-Plane conformance tests;
5. audio comparison tests against documented hardware captures.

The current NOP test is level 2 smoke coverage only. It is not a firmware boot test and does not demonstrate sound generation or successful Morpheus emulation.

## Next reverse-engineering targets

1. Obtain authoritative board photographs, service documentation, and verified chip markings.
2. Establish the CPU ROM layout and reset/interrupt behavior from evidence.
3. Identify sound ASIC roles and their host interfaces before writing a memory map.
4. Separate the effects DSP board from the main synthesis signal path.
5. Build hardware-reference tests before implementing any undocumented register behavior.
