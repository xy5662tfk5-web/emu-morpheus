# Morpheus preset SysEx format

This document records the preset-dump format used by the AU's metadata
browser. It is a file/protocol parser only; it is not a firmware loader,
sample-ROM loader, or implementation of the Morpheus synthesis engine.

## Preset Data frame

A complete preset-data message is 566 bytes:

| Offset | Meaning |
|---:|---|
| 0 | SysEx start: `F0` |
| 1 | E-MU manufacturer ID: `18` |
| 2 | Morpheus product ID: `0C` |
| 3 | Device ID: 0–15 |
| 4 | Command: `01` (Preset Data) |
| 5 | Data format version: `01` |
| 6–7 | Preset number, 14-bit, low 7 bits first |
| 8–563 | 278 fourteen-bit parameter words, each low MIDI byte then high MIDI byte |
| 564 | Checksum |
| 565 | SysEx end: `F7` |

All data bytes between the header and checksum must be seven-bit MIDI bytes.
The checksum is the low seven bits of the sum of the parameter-data bytes at
offsets 8 through 563 inclusive. The header, preset number, checksum byte,
and F7 are not included in that sum.

The preset name occupies the first twelve parameter words. Each character is
the low seven bits of its word; the corresponding high seven-bit part is not
required to be zero. Printable ASCII is expected.

## Parser behavior

- Split concatenated SysEx messages by F0/F7 framing.
- Accept only E-MU manufacturer 18, Morpheus product 0C, command 01,
  version 01, valid device IDs, and the fixed 566-byte preset frame.
- Verify that all parameter bytes are seven-bit and that the checksum matches.
- Preserve the preset's 14-bit object number and decode its twelve-character
  name.
- Count malformed frames separately from other/unsupported SysEx messages.
- Do not reinterpret Proteus product ID 04 messages as Morpheus presets.
- Hyperpreset and midimap dumps are deliberately outside this preset-bank
  parser; their command IDs and frame sizes differ.

## Evidence and implementation boundary

The format follows E-MU's Morpheus Operation Manual, MIDI Specification
chapter (printed pages 225–250). The open-source
[lentferj/emorphed](https://github.com/lentferj/emorphed) project is used as
an independent cross-check: its `emu/messages.py` transcribes the manual's
14-bit word and command definitions, while `emu/dumps.py` validates
fixed-size messages and checksums against dumps from real instruments.
That project is GPL-2.0-or-later. This repository records the protocol facts
and maintains its own C++ implementation; it does not import emorphed code.

This parser only makes preset names and object numbers trustworthy. It does
not decode all 278 parameter meanings, load sample data, execute firmware,
or produce Morpheus audio. Those remain separate engineering tasks.
