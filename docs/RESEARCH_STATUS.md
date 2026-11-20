# Research status

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

## Candidate CPU core

[Moira](https://github.com/dirkwhoffmann/Moira) is a possible Motorola 68k core. Before integration, verify the exact 68020 support/configuration, bus callback model, exception behavior, timing assumptions, and upstream license/version. CPU integration alone will not produce sound.

## Test policy

Tests must distinguish:
1. scaffold/interface tests;
2. CPU instruction and bus tests;
3. firmware boot tests;
4. sound-engine and Z-Plane conformance tests;
5. audio comparison tests against documented hardware captures.

Never label level 1 as successful Morpheus emulation.
