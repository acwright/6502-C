# VDP assessment: 6502-C

> An outline, not a plan. The detailed plan for this repository goes in `VDP-PLAN.md`,
> written in a session of its own. Surveyed 2026-09-16 across the whole workspace.

## The change

The ACE moves from a Pico9918 running stock TMS9918A firmware to the **6502-PICOVDP**
(`6502-PICOVDP/SPEC.md`) on PICO9918 PRO v2.0 hardware, running **BIOS 2.x**. Everything
else stays where it is: COB, DEV, KIM, VCS, PicoCalc, and any ACE whose card cannot be
reflashed (RP2040 pico9918 v1.0–1.3). Those keep the stock firmware and **BIOS 1.x (1.5)**.

- **Legacy** in these documents means TMS9918A + BIOS 1.x. **VDP** means PICOVDP + BIOS 2.x.
- **Compatibility runs one way.** The PICOVDP's legacy submode runs Text and Graphics I
  programs unchanged, so BIOS 1.5 and existing cartridges run on it. Graphics II and
  Multicolor fall back to Graphics I and draw garbage. Register writes above 7 no longer
  alias, so F18A tricks break. Sprites per line are 16 by default, not 4. Nothing written
  for the VDP runs on a TMS9918A.
- **BIOS 2.0 is assumed to be:** BIOS 1.5, plus the NVRAM save slots in
  `6502-BIOS/PLAN.md`, plus the VDP work in `6502-EMULATOR`'s
  `docs/handoff/6502-BIOS.md` (branch `v3-vdp`). Existing jump-table addresses stay put.
  A later BIOS redesign may revise this.

## Decisions already made

- No new repositories.
- **6502-EMULATOR** makes the video card an option (TMS9918A or PICOVDP): one app, one
  site. It also publishes a frozen 2.6.9 web build at a versioned path for the legacy docs.
- **6502-DOCS** is versioned: legacy docs are frozen at `/6502-DOCS/v1/`, and the main
  site is rewritten for the VDP.
- **6502-BIOS** gets a `v1.x` maintenance branch; `main` becomes 2.x.
- **Assembly and C projects** get a VDP include chosen by a build option, not branches.
- **EhBASIC and vc83basic** stay 1.x. **PicoCalc** stays legacy. **The YouTube series**
  teaches the legacy VDP and mentions the new features.

## Order across the workspace

1. **6502-PICOVDP:** firmware proven on the PRO (its Phases 9–11). This gates the
   hardware switch, not the software work.
2. **6502-EMULATOR:** frozen 2.6.9 web build at `/6502-EMULATOR/v2/`.
3. **6502-DOCS:** `v1` branch published at `/6502-DOCS/v1/`, embeds pinned to step 2.
4. **6502-EMULATOR:** `v3-vdp` merged, with the card as an option; tagged 3.x.
5. **6502-BIOS:** `v1.x` cut; 2.0 built on `main`. This can start any time, because the
   `v3-vdp` emulator already runs the PICOVDP.
6. **6502-ASM** sets the VDP include convention. 6502-CRT, 6502-PRG, 6502-BIN and 6502-C
   follow it.
7. The emulator bundles BIOS 2.0. 6502-DOCS `main` is rewritten. 6502-ACE, bastok,
   WIZARDSLAB, 6502-EHBASIC, vc83basic and 6502-ASSEMBLY follow.

---

## This repository's role

cc65 C support for the ACE:
- `6502.h`, the C mirror of `6502.inc`, "section for section, name for name".
- `6502.inc` for ca65 parts.
- `lib/`, which builds `6502.lib` (crt0, conio, Kernal wrappers).
- `HelloWorld` and `HelloWorldCart` programs.
- `tests/`: `parity.py` checks `6502.h` against `6502.inc`, and `regcheck.c`.

## Where it stands

- Its `6502.inc` is the settled legacy include, byte-identical across the workspace and
  checked against the BIOS v1.5 build (see 6502-ASM's assessment). `6502.h` gained the
  one constant added in that pass (`SC_CMD_RXIRQ_OFF`). `make check` passes: 116
  addresses, 0 mismatched.
- TMS9918 vocabulary in the public API:
  - `TMS_*` colour constants (used by `HelloWorld`: `VideoSetColor(TMS_LT_GREEN << 4 | …)`).
  - `IO_MODE_VIDEO` "goes to the TMS9918".
  - `VC_DATA`/`VC_REG`/`VC_STATUS` at `$9C00`/`$9C01` only.
- `lib/video.s` has the only direct VDP port access in the C/ASM templates: a
  vertical-blank wait that polls `VC_STATUS`. That works on both cards, because `STAT0`
  b7 is set every frame. On the PICOVDP, though, a status read is only `STAT0` if nothing
  left another status register selected.

## Work outline

**Blocked on 6502-ASM's convention** (and on BIOS 2.0 for Kernal entries).

1. **Includes.** Keep the legacy `6502.inc` as it is. Add the VDP include as 6502-ASM
   defines it, and a matching C header (working name `6502-VDP.h`), selected by the same
   build option.
2. **Parity.** Extend `tests/parity.py` to check both pairs (`6502.h`↔`6502.inc`, and the
   VDP header against the VDP include).
3. **Library.**
   - Wrappers for BIOS 2.0's new entries: NVRAM slots, `WaitVBlank`, the VDP entry points.
   - Decide between one library with extra modules, or a VDP build of the library (for
     example `6502-vdp.lib`), so legacy programs can't link calls a legacy ROM lacks.
   - In VDP builds, the vblank wait in `lib/video.s` could call BIOS 2.0's `WaitVBlank`.
4. **Names.** Keep `TMS_*` for the legacy header. The VDP header needs palette-oriented
   names (row 0 keeps the same sixteen indices) and C names for the registers `$08`–`$7F`.
5. **Programs and README.** Build `HelloWorld`/`HelloWorldCart` both ways; `make run`
   selects the emulator card; the README explains which header to use.

## Linked repositories

| Repository | Path | Why |
|---|---|---|
| 6502-ASM | `~/Developer/Assembly/6502-ASM` | Defines the include and build-option convention; the legacy `6502.inc` is kept identical with its copy |
| 6502-BIOS | `~/Developer/Assembly/6502-BIOS` | BIOS 2.0 jump table the library wraps |
| 6502-PICOVDP | `~/Developer/C/6502-PICOVDP` | `SPEC.md` §4–§6 for register and status names |
| 6502-EMULATOR | `~/Developer/NodeJS/6502-EMULATOR` | Card flag for `make run` |
| 6502-DOCS | `~/Developer/NodeJS/6502-DOCS` | `docs/crossdev/cc65.md` documents this repository |

## Questions for VDP-PLAN.md

1. One library or two.
2. Header naming for the VDP colour and register constants.
3. Whether conio gains anything from the VDP (hardware scroll arrives for free through the
   Kernal).
