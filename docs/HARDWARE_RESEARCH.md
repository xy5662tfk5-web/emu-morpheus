# Morpheus hardware research notes

This file separates published observations from assumptions. A published description of a chip or signal path is useful evidence, but it does not reveal undocumented register maps or firmware protocols.

## Confirmed from contemporary hardware reporting

The December 1993 *Sound On Sound* teardown/review reports:

- Motorola 68020 as the main processor.
- Five large AMI application-specific ICs on the main PCB.
- An Analog Devices ADSP-2115 on a small piggyback PCB, described as producing Morpheus digital effects.
- Six Analog Devices AD1860 oversampling DACs.
- 8 MB sound memory, 16-bit linear, 39 kHz.
- The ADSP-2115 is therefore associated with the effects subsystem in this report; it should not be assumed to be the main Z-Plane or voice-generation processor.

Source: Martin Russ, “Morphology,” *Sound On Sound*, December 1993:
https://www.muzines.benhall.co.uk/articles/morphology/10809

## Published synthesis architecture

The November 1993 *Music Technology* preview describes the Z-Plane concept as a three-dimensional filter cube:

- Eight filter frames form the cube's corners.
- The X and Y axes are used for note-number and velocity-related transforms.
- The Z axis is the continuous morph dimension during a note.
- The article describes filter frames built from eight 14-pole filters and discusses two configurations: seven second-order all-pole low-pass sections in series, or a two-pole resonant low-pass plus six parametric EQ sections.

Source: Simon Trask, “E-mu Morpheus,” *Music Technology*, November 1993:
https://www.muzines.co.uk/articles/e-mu-morpheus/7649

The *Sound On Sound* review describes the voice path as oscillator/sample source, envelope, per-oscillator filter, cross-fading, panning, and output. A preset can contain two independently processed oscillator voices. The operation manual's signal-flow diagram should be used alongside this editorial description when translating the conceptual architecture into software modules.

Source: Martin Russ, “Morphology,” *Sound On Sound*, December 1993:
https://www.muzines.benhall.co.uk/articles/morphology/10809

Official E-MU Morpheus Operation Manual (1993, FI420 Rev. C):
https://medias.audiofanzine.com/files/e-mu-morph-om-480525.pdf

## Engineering implications

1. Keep the effects DSP path separate from the main voice/synthesis path until board-level evidence proves their exact coupling.
2. Model two independent voice lanes per preset before their documented crossfade/pan/output stages.
3. Treat Z-Plane as a time-varying multi-frame filter system, not as a single cutoff/resonance filter or a generic two-filter crossfade.
4. Do not invent a CPU address map, ASIC register interface, clock rate, or undocumented data format.
5. Build an evidence table for each chip: board location, marking, role, bus connections, reset behavior, clocks, and known firmware references.

## Unresolved questions

- Exact part markings and individual roles of the five AMI ASICs.
- Which ASIC(s) implement sample address generation, interpolation, voice mixing, and Z-Plane filtering.
- CPU memory map, ROM organization, boot sequence, interrupt sources, and ASIC register protocol.
- Sample-ROM banking/addressing and exact per-voice playback behavior.
- Whether any internal firmware or microcode images can be obtained and analyzed lawfully.
- Exact interaction between the main synthesis engine and the ADSP-2115 effects board.

## Current code boundary

The integrated Moira 68020 core is connected to an abstract byte-addressed bus. That is intentionally not presented as a Morpheus memory map. The next useful implementation is a testable machine/bus abstraction backed by verified address-decoding evidence, not guessed hardware constants.
