/* =============================================================================
 *   6502.h — AC6502 system header for C programs
 * =============================================================================
 *
 *   CPU:       WDC 65C02S
 *   BIOS:      v1.6
 *   Compiler:  cc65
 *
 *   The C counterpart of 6502.inc.  Include it to reach the Kernal routines,
 *   the IO hardware registers, the system variables and the constants that
 *   go with them.  The names, the addresses and the section order all match
 *   6502.inc, so the two can be read side by side.
 *
 *   Hardware registers and system variables are exposed as volatile lvalues,
 *   so they read and assign like ordinary variables:
 *
 *       VideoSetColor(TMS_LT_GREEN << 4 | TMS_BLACK);
 *       if (HW_PRESENT & HW_SID) { Beep(); }
 *       RAM_BANK_L = 3;                     ~ select banked RAM page 3
 *
 *   MEMORY MAP (as a C program sees it)
 *   -----------------------------------
 *   $003A-$00FF   Zero page available to the program — the cc65 runtime
 *                 takes the first 26 bytes of it (see 6502.cfg)
 *   $0100-$01FF   CPU stack (hardware; untouched by the C runtime)
 *   $0800-$77FF   Program: startup, code, rodata, data, bss, then the heap
 *   $7800-$7FFF   C parameter stack, growing down from $8000
 *
 *   Everything outside that is the BIOS's, and 6502.inc documents it in full.
 *
 *   CALLING CONVENTION
 *   ------------------
 *   Every routine below is a thin __fastcall__ wrapper around the Kernal jump
 *   table entry of the same name, so the cost of calling one from C is a JSR
 *   and a register shuffle.  Routines that report failure in the carry flag
 *   return 0 for success and 1 for error, so they read naturally as:
 *
 *       if (FsLoadFileAddr("DATA.BIN", buffer) != 0) { ... }
 *
 *   STDIO
 *   -----
 *   printf(), puts() and putchar() work: the library's write() is wired to
 *   Chrout, and '\n' is expanded to CR+LF on the way out.  They are not free
 *   — printf() alone is a couple of KB — so Chrout and PrintStr remain the
 *   cheap way to get characters onto the screen.
 *
 * =============================================================================
 */

#ifndef _AC6502_H
#define _AC6502_H


/* __fastcall__ is a cc65 keyword, not standard C, and an editor's parser
 * stops dead at the first declaration that uses one.  It only means anything
 * when cc65 is the one compiling, so define it away for everybody else.
 * .vscode/c_cpp_properties.json does the same for cc65's own headers, which
 * use it several hundred times over. */
#ifndef __CC65__
#define __fastcall__
#endif


/* =============================================================================
 *   SECTION 1: ZERO PAGE POINTERS
 * =============================================================================
 * These zero page locations are used by the Kernal.  A C program normally has
 * no reason to touch them — the wrappers below set STR_PTR, CF_BUF_PTR and
 * CF_LBA for you — but they are here for code that calls the Kernal directly.
 *
 * $3A-$FF is the free range, and the cc65 runtime lives at the bottom of it.
 */

#define STR_PTR             (*(unsigned char **)0x02)     /* String pointer (FsLoadFile, FsSaveFile, FsDeleteFile, ...) */
#define CF_BUF_PTR          (*(unsigned char **)0x24)     /* CF sector data buffer pointer */
#define CF_LBA              (*(volatile unsigned long *)0x26) /* CF 28-bit LBA address */
#define XFER_PTR            (*(unsigned char **)0x2A)     /* XModem transfer data pointer */


/* =============================================================================
 *   SECTION 2: SYSTEM VARIABLES ($0300-$03FF)
 * =============================================================================
 */

#define IRQ_PTR             (*(void (**)(void))0x0300)    /* IRQ handler vector (default: internal Irq handler) */
#define BRK_PTR             (*(void (**)(void))0x0302)    /* BRK handler vector (default: enters Monitor) */
#define NMI_PTR             (*(void (**)(void))0x0304)    /* NMI handler vector (default: RTI) */

#define IO_MODE             (*(volatile unsigned char *)0x0306)  /* Console output mode (bit 0: 0=video, 1=serial) */
#define VID_CURSOR_X        (*(volatile unsigned char *)0x0307)  /* Video cursor column (0-39) */
#define VID_CURSOR_Y        (*(volatile unsigned char *)0x0308)  /* Video cursor row (0-23) */

#define HW_PRESENT          (*(volatile unsigned char *)0x030D)  /* Hardware present bitmask (set during boot probe) */
#define CF_DISK             (*(volatile unsigned char *)0x030F)  /* Current CF disk bank (0-255) */

/* HW_PRESENT bit definitions (bit order matches IO slot numbers): */
#define HW_RAM_L            0x01          /* Bit 0: RAM card low  (IO 1, $8000-$83FF) */
#define HW_RAM_H            0x02          /* Bit 1: RAM card high (IO 2, $8400-$87FF) */
#define HW_RTC              0x04          /* Bit 2: RTC card      (IO 3, DS1511Y) */
#define HW_CF               0x08          /* Bit 3: Storage card  (IO 4, CompactFlash) */
#define HW_SC               0x10          /* Bit 4: Serial card   (IO 5, R65C51) */
#define HW_GPIO             0x20          /* Bit 5: GPIO card     (IO 6, 65C22 VIA) */
#define HW_SID              0x40          /* Bit 6: Sound card    (IO 7, SID/ARMSID) */
#define HW_VID              0x80          /* Bit 7: Video card    (IO 8, TMS9918/pico9918) */

/* IO_MODE values: */
#define IO_MODE_VIDEO       0x00          /* Console output goes to the TMS9918 */
#define IO_MODE_SERIAL      0x01          /* Console output goes to the R65C51 */

/* BRK register save area (populated by the BRK handler before entering Monitor): */
#define BRK_P               (*(volatile unsigned char *)0x0310)  /* Saved processor status (P) */
#define BRK_PCL             (*(volatile unsigned char *)0x0311)  /* Saved PC low byte (BRK instruction + 2) */
#define BRK_PCH             (*(volatile unsigned char *)0x0312)  /* Saved PC high byte */
#define BRK_A               (*(volatile unsigned char *)0x0313)  /* Saved accumulator */
#define BRK_X               (*(volatile unsigned char *)0x0314)  /* Saved X register */
#define BRK_Y               (*(volatile unsigned char *)0x0315)  /* Saved Y register */
#define BRK_SP              (*(volatile unsigned char *)0x0316)  /* Saved stack pointer */

#define XFER_REMAIN         (*(volatile unsigned int *)0x0317)   /* XModem byte count */
#define XFER_IO_SAVE        (*(volatile unsigned char *)0x0319)  /* Saved IO_MODE during XModem transfer */

#define RTC_BUF_CENT        (*(volatile unsigned char *)0x030B)  /* Century for RtcReadDate / RtcWriteDate */

#define BOOT_VECTOR         (*(void (**)(void))0x035B)           /* Boot redirect address (0 = normal boot) */
#define FS_IO_ADDR          (*(unsigned char **)0x037F)          /* Load/save address for FsLoadFileAddr / FsSaveFileAddr */
#define FS_FILE_SIZE        (*(volatile unsigned int *)0x034A)   /* File size in bytes */
#define PRG_IMAGE_END       (*(volatile unsigned int *)0x038E)   /* End address of a loaded program image (0 = none) */
#define NV_ID               (*(volatile unsigned char *)0x0390)  /* Owner ID input for the NvWrite Kernal entry (the C wrapper sets it) */


/* =============================================================================
 *   SECTION 3: KEY MEMORY LOCATIONS
 * =============================================================================
 */

#define INPUT_BUFFER        ((volatile unsigned char *)0x0200)   /* 256-byte ring buffer (keyboard/serial input) */
#define PROGRAM_START       ((unsigned char *)0x0800)            /* User programs load here */
#define MEMORY_TOP          ((unsigned char *)0x8000)            /* First address past usable RAM */


/* =============================================================================
 *   SECTION 4: KERNAL JUMP TABLE ($A000-$A0FF)
 * =============================================================================
 * One wrapper per entry point, named exactly as 6502.inc names it.  Each is a
 * __fastcall__ function in 6502.lib that shuffles arguments into the registers
 * the routine wants and JSRs the jump table.
 *
 * Routines that report failure in the carry flag return `unsigned char`:
 * 0 = success, 1 = error.
 *
 * Not wrapped: XModemLoad / XModemSave and StReadSector / StWriteSector.  Their
 * arguments are zero page pointers the caller sets up and the routine advances
 * in place, which is a poor fit for a C prototype — drive them through the
 * XFER_PTR / CF_BUF_PTR / CF_LBA macros above, or from assembly via 6502.inc.
 */

/* --- Character I/O --- */

void __fastcall__ Chrout(char c);
/* Output a character to the console selected by IO_MODE.
 * CR ($0D), LF ($0A), BS ($08) and BEL ($07) are interpreted in video mode. */

int Chrin(void);
/* Poll for an input character.  Returns the character, or -1 if none is
 * waiting.  Does not block; the character is echoed via Chrout. */

void __fastcall__ WriteBuffer(char c);
/* Push a byte into the input ring buffer. */

char ReadBuffer(void);
/* Pull a byte out of the input ring buffer. */

unsigned char BufferSize(void);
/* Number of unread bytes in the input buffer. */

/* --- IO Mode --- */

void __fastcall__ SetIOMode(unsigned char mode);
/* Select the console: IO_MODE_VIDEO or IO_MODE_SERIAL. */

unsigned char GetIOMode(void);
/* The active console mode. */

/* --- Video (TMS9918 / pico9918) --- */

void InitVideo(void);
/* Initialize the TMS9918 to 40x24 text and reload the character set.  Restores
 * text mode in full after a program has switched the VDP to graphics. */

void VideoClear(void);
/* Fill the screen with spaces and put the cursor back at (0,0). */

void __fastcall__ VideoPutChar(char c);
/* Write a character at the cursor without advancing the cursor. */

void __fastcall__ VideoSetCursor(unsigned char col, unsigned char row);
/* Move the cursor.  col 0-39, row 0-23. */

unsigned int VideoGetCursor(void);
/* Cursor position packed as (row << 8) | col — see VID_COL() and VID_ROW().
 * VID_CURSOR_X and VID_CURSOR_Y hold the same two values. */

#define VID_COL(pos)        ((unsigned char)((pos) & 0xFF))
#define VID_ROW(pos)        ((unsigned char)((pos) >> 8))

void VideoScroll(void);
/* Scroll up one line and clear the bottom row. */

void __fastcall__ VideoSetColor(unsigned char color);
/* Set the text color: (foreground << 4) | background, TMS_* constants. */

void __fastcall__ VideoChroutRaw(char c);
/* Write a character to video with no control-code handling — always a glyph,
 * never a CR, LF, BS or BEL. */

/* --- Sound (SID / ARMSID) --- */

void InitSID(void);
/* Clear every SID register and set the master volume to 15. */

void Beep(void);
/* A short ~475 Hz beep. */

void __fastcall__ SidPlayNote(unsigned char voice, unsigned int freq);
/* Play a note on voice 0-2 using the SID_NOTE_* frequencies below.  Triangle
 * waveform, ADSR A=0 D=9 S=F R=8 — full sustain holds the note until it is
 * gated off with SidSilence(). */

void SidSilence(void);
/* Gate off all three voices and let the release ring them out. */

void __fastcall__ SidSetVolume(unsigned char volume);
/* Master volume, 0-15. */

/* --- Filesystem (disk-banked, 16-file directory per disk) --- */
/* Filenames are 8.3, case-insensitive, NUL-terminated: "PROGRAM.PRG".
 * Every routine here works against the disk bank CF_DISK selects, and uses
 * $0600-$07FF as a sector buffer. */

unsigned char __fastcall__ FsLoadFile(const char *name);
/* Load a file to the address recorded in its directory entry. */

unsigned char __fastcall__ FsSaveFile(const char *name);
/* Save a file from the address recorded in its directory entry. */

unsigned char __fastcall__ FsDeleteFile(const char *name);
/* Remove a directory entry. */

unsigned char __fastcall__ FsLoadFileAddr(const char *name, void *addr);
/* Load a file to an address of your choosing.  On success FS_FILE_SIZE holds
 * the number of bytes read. */

unsigned char __fastcall__ FsSaveFileAddr(const char *name, const void *addr,
                                          unsigned int size);
/* Write `size` bytes from `addr` to a named file. */

unsigned char FsFormatDisk(void);
/* Zero the current disk's directory — all 16 entries, no confirmation. */

void __fastcall__ FsSetDisk(unsigned char disk);
/* Select the current CF disk bank, 0-255. */

unsigned char FsGetDisk(void);
/* The current CF disk bank. */

void FsPrintDisk(void);
/* Print "DISK n" and a CRLF to the console. */

/* --- Keyboard / GPIO --- */

void InitKB(void);
/* Configure the VIA ports for keyboard input and enable the CB1/CA1 IRQs. */

unsigned char ReadJoystick1(void);
unsigned char ReadJoystick2(void);
/* Read a joystick port raw.  Both ports are ACTIVE LOW, so an untouched stick
 * reads 0xFF and a held control is a 0 bit:
 *
 *     if (!(ReadJoystick1() & JOY_UP)) { ... }
 *
 * Neither routine checks HW_PRESENT — with no GPIO card fitted they read a
 * floating bus, so test HW_GPIO first if the card may be absent. */

#define JOY_A               0x01          /* Bit 0 */
#define JOY_B               0x02          /* Bit 1 */
#define JOY_X               0x04          /* Bit 2 */
#define JOY_Y               0x08          /* Bit 3 */
#define JOY_UP              0x10          /* Bit 4 */
#define JOY_DOWN            0x20          /* Bit 5 */
#define JOY_LEFT            0x40          /* Bit 6 */
#define JOY_RIGHT           0x80          /* Bit 7 */

void KBDisable(void);
/* Release both keyboard encoders and wait ~200 us for the ports to settle, so
 * GPIO_PORTA / GPIO_PORTB can be read directly. */

void KBEnable(void);
/* Re-enable both keyboard encoders. */

/* --- Serial (R65C51) --- */

void InitSC(void);
/* Initialize the serial card: 8-N-1, 19200 baud, RX IRQ enabled. */

void __fastcall__ SerialChrout(char c);
/* Send a character straight out the serial port, whatever IO_MODE says.
 * Blocks until the TX register is empty. */

/* --- RTC (DS1511Y) --- */
/* All fields are binary, not BCD — the Kernal converts. */

typedef struct {
    unsigned char hours;                  /* 0-23 */
    unsigned char minutes;                /* 0-59 */
    unsigned char seconds;                /* 0-59 */
} RtcTime;

typedef struct {
    unsigned char day;                    /* 1-31 */
    unsigned char month;                  /* 1-12 */
    unsigned char year;                   /* 0-99 */
    unsigned char century;                /* 0-39, via RTC_BUF_CENT */
} RtcDate;

void __fastcall__ RtcReadTime(RtcTime *time);
void __fastcall__ RtcWriteTime(const RtcTime *time);
void __fastcall__ RtcReadDate(RtcDate *date);
void __fastcall__ RtcWriteDate(const RtcDate *date);

unsigned char __fastcall__ RtcReadNVRAM(unsigned char addr);
void __fastcall__ RtcWriteNVRAM(unsigned char addr, unsigned char value);
/* The DS1511Y's 256 bytes of battery-backed NVRAM.  Neither checks
 * HW_PRESENT — test HW_RTC first if the card may be absent. */

/* --- NVRAM Save Slots (DS1511Y) (BIOS v1.6) --- */
/* The same 256 bytes as 16 slots of 14 payload bytes each, with an owner ID
 * and a checksum per slot (layout in 6502.inc and the BIOS README).  All six
 * check for the RTC themselves.  On a BIOS older than 1.6 the entries are
 * reserved slots that do nothing, so check KernalVersion() first. */

unsigned int __fastcall__ NvStat(unsigned char slot);
/* Slot state packed as (owner << 8) | status — see NV_STATUS() and
 * NV_OWNER().  The status is NV_EMPTY, NV_VALID or NV_BAD, or NV_ERROR if
 * there is no RTC or the slot is not 0-15. */

unsigned int __fastcall__ NvRead(unsigned char slot, void *buf);
/* Copy a slot's NV_SLOT_DATA payload bytes to buf, packed like NvStat.  Only
 * an NV_VALID slot is copied; on any other status buf is left untouched, and
 * the status says why — no save yet, or a damaged one. */

unsigned char __fastcall__ NvWrite(unsigned char slot, unsigned char owner,
                                   const void *buf);
/* Write NV_SLOT_DATA bytes from buf as owner (1-255).  Returns 0 on success,
 * 1 if there is no RTC, the slot is not 0-15, or owner is 0 (nothing is
 * written — use NvErase). */

unsigned char __fastcall__ NvErase(unsigned char slot);
/* Zero all 16 bytes of a slot.  Returns 0 on success, 1 on no RTC or a bad
 * slot. */

unsigned char __fastcall__ NvFind(unsigned char owner);
/* The lowest slot owner holds, valid or damaged, or NV_NONE.  Owner 0 finds
 * the lowest free slot. */

unsigned char NvFormat(void);
/* Erase all 16 slots.  Returns 0 on success, 1 on no RTC. */

#define NV_STATUS(v)        ((unsigned char)((v) & 0xFF))
#define NV_OWNER(v)         ((unsigned char)((v) >> 8))

/* --- CompactFlash Storage --- */

unsigned char StWaitReady(void);
/* Wait for the CF card to go ready.  Returns 0 when it is, 1 on timeout — or
 * immediately 1 if the boot probe found no card. */

/* --- Timing --- */

void __fastcall__ SysDelay(unsigned int centiseconds);
/* Block for centiseconds x 10 ms.  Uses the VIA timer if a GPIO card is
 * fitted, otherwise a calibrated software loop. */

/* --- System --- */

unsigned int KernalVersion(void);
/* BIOS version, low byte major and high byte minor — see the accessors. */

#define VER_MAJOR(v)        ((unsigned char)((v) & 0xFF))
#define VER_MINOR(v)        ((unsigned char)((v) >> 8))

/* --- General Output Utilities (console routed by IO_MODE via Chrout) --- */

void __fastcall__ PrintStr(const char *str);
/* Print a NUL-terminated string.  No newline, no formatting, no cost beyond
 * one Chrout per character — the cheap way to put text on the screen. */

void PrintCRLF(void);
/* Print CR ($0D) then LF ($0A). */

void __fastcall__ PrintDecU16(unsigned int value);
/* Print an unsigned 16-bit value as decimal, no leading zeros.  Clobbers
 * FS_FILE_SIZE, so read that before calling this. */


/* =============================================================================
 *   SECTION 4b: C HELPERS
 * =============================================================================
 * Not part of the jump table — small conveniences built on top of it.
 */

char ChrinWait(void);
/* Poll Chrin until a character arrives, then return it.  Blocks. */

void waitvsync(void);
/* Block until the VDP finishes a frame, so that a screen update happens in
 * the blanking interval rather than partway down the picture.  Returns at
 * once if no video card is fitted.
 *
 * Reading the TMS9918 status register is what clears its frame flag, so this
 * competes with anything else that reads it.  The wait gives up after about
 * five frames rather than blocking forever, which turns a flag consumed
 * elsewhere into a dropped sync instead of a hung machine. */


/* =============================================================================
 *   SECTION 4c: THE C LIBRARY
 * =============================================================================
 * Parts of cc65's own library that need a machine underneath them before they
 * will link.  Nothing here is declared by 6502.h — include the standard header
 * and the support comes out of 6502.lib.
 *
 *   <stdio.h>   printf, puts, putchar and the rest, via write() -> Chrout.
 *
 *   <conio.h>   The full console API: clrscr, gotoxy, cputs, cprintf, cgetc,
 *               kbhit, cputhex8/16, chline, cvline.  Output follows IO_MODE,
 *               so it works on the serial console too.  Three calls do not
 *               fit a TMS9918 and behave differently to a VIC-II — see the
 *               comment at the top of lib/conio.s before relying on
 *               textcolor(), bgcolor() or revers().
 *
 *   <time.h>    time(), localtime(), gmtime(), mktime(), strftime(), ctime()
 *               — the whole header, backed by the DS1511Y, which keeps time
 *               across a power cycle.  clock() is NOT available: it needs a
 *               free-running tick this machine has no counter for.
 *
 * The RTC is read as whatever _tz says it holds; the default all-zero _tz
 * means UTC.  See lib/gettime.c.
 */


/* =============================================================================
 *   SECTION 5: IO HARDWARE REGISTERS
 * =============================================================================
 */

/* --- Banked RAM Low (IO 1: $8000-$83FF) --- */
/* 256 banks of ~1 KB.  Write the bank number to the latch, then use the
 * window: RAM_BANK_L = 7; RAM_DATA_L[0] = 0x42; */
#define RAM_DATA_L          ((volatile unsigned char *)0x8000)   /* R/W data window ($8000-$83FE) */
#define RAM_BANK_L          (*(volatile unsigned char *)0x83FF)  /* Write-only bank latch (0x00-0xFF) */

/* --- Banked RAM High (IO 2: $8400-$87FF) --- */
#define RAM_DATA_H          ((volatile unsigned char *)0x8400)   /* R/W data window ($8400-$87FE) */
#define RAM_BANK_H          (*(volatile unsigned char *)0x87FF)  /* Write-only bank latch (0x00-0xFF) */

/* --- RTC (DS1511Y) (IO 3: $8800-$881F) --- */
#define RTC_SEC             (*(volatile unsigned char *)0x8800)  /* Seconds      (BCD, 00-59) */
#define RTC_MIN             (*(volatile unsigned char *)0x8801)  /* Minutes      (BCD, 00-59) */
#define RTC_HR              (*(volatile unsigned char *)0x8802)  /* Hours        (BCD, 00-23, 24h mode) */
#define RTC_DAY             (*(volatile unsigned char *)0x8803)  /* Day of week  (1-7) */
#define RTC_DATE            (*(volatile unsigned char *)0x8804)  /* Day of month (BCD, 01-31) */
#define RTC_MON             (*(volatile unsigned char *)0x8805)  /* Month        (BCD, 01-12) */
#define RTC_YR              (*(volatile unsigned char *)0x8806)  /* Year         (BCD, 00-99) */
#define RTC_CENT            (*(volatile unsigned char *)0x8807)  /* Century      (BCD, 00-39) */
#define RTC_ALARM_SEC       (*(volatile unsigned char *)0x8808)  /* Alarm seconds */
#define RTC_ALARM_MIN       (*(volatile unsigned char *)0x8809)  /* Alarm minutes */
#define RTC_ALARM_HR        (*(volatile unsigned char *)0x880A)  /* Alarm hours */
#define RTC_ALARM_DATE      (*(volatile unsigned char *)0x880B)  /* Alarm day of month */
#define RTC_WD_L            (*(volatile unsigned char *)0x880C)  /* Watchdog low */
#define RTC_WD_H            (*(volatile unsigned char *)0x880D)  /* Watchdog high */
#define RTC_CTRL_A          (*(volatile unsigned char *)0x880E)  /* Control Register A */
#define RTC_CTRL_B          (*(volatile unsigned char *)0x880F)  /* Control Register B */
#define RTC_RAM_ADDR        (*(volatile unsigned char *)0x8810)  /* NVRAM address register */
#define RTC_RAM_DATA        (*(volatile unsigned char *)0x8813)  /* NVRAM data register */

#define RTC_CTRL_B_TE       0x80          /* Transfer Enable — 1 = counters update the user registers once a second, 0 = inhibited */
#define RTC_CTRL_B_BME      0x20          /* Burst Mode Enable — 1 = RTC_RAM_ADDR auto-increments on each access of RTC_RAM_DATA */

/* The month register's upper 3 bits are oscillator/SQW control, not month
 * data.  Mask with RTC_MON_MASK on read and preserve them on write. */
#define RTC_MON_MASK        0x1F          /* Month value occupies the low 5 bits */
#define RTC_MON_EOSC        0x80          /* Oscillator start/stop (0 = running) */
#define RTC_MON_E32K        0x40          /* 32 kHz square-wave enable (1 = SQW disabled) */
#define RTC_MON_BB32        0x20          /* Battery-backup 32 kHz SQW enable */

/* --- CompactFlash / Storage (IO 4: $8C00-$8C07) --- */
/* 8-bit IDE (True IDE) mode.  The BIOS issues Set Features $01 at boot to
 * enable 8-bit data transfers. */
#define ST_DATA             (*(volatile unsigned char *)0x8C00)  /* R/W  — Data register */
#define ST_ERROR            (*(volatile unsigned char *)0x8C01)  /* Read — Error register */
#define ST_FEATURE          (*(volatile unsigned char *)0x8C01)  /* Write— Feature register */
#define ST_SECT_CNT         (*(volatile unsigned char *)0x8C02)  /* R/W  — Sector count */
#define ST_LBA_0            (*(volatile unsigned char *)0x8C03)  /* R/W  — LBA bits 0-7 */
#define ST_LBA_1            (*(volatile unsigned char *)0x8C04)  /* R/W  — LBA bits 8-15 */
#define ST_LBA_2            (*(volatile unsigned char *)0x8C05)  /* R/W  — LBA bits 16-23 */
#define ST_LBA_3            (*(volatile unsigned char *)0x8C06)  /* R/W  — LBA bits 24-27 + drive/mode */
#define ST_STATUS           (*(volatile unsigned char *)0x8C07)  /* Read — Status register */
#define ST_CMD              (*(volatile unsigned char *)0x8C07)  /* Write— Command register */

/* CF Status Register bits: */
#define ST_STATUS_ERR       0x01          /* Error occurred */
#define ST_STATUS_IDX       0x02          /* Index mark */
#define ST_STATUS_CORR      0x04          /* Corrected data */
#define ST_STATUS_DRQ       0x08          /* Data request — ready for data transfer */
#define ST_STATUS_DSC       0x10          /* Drive seek complete */
#define ST_STATUS_DWF       0x20          /* Drive write fault */
#define ST_STATUS_RDY       0x40          /* Drive ready */
#define ST_STATUS_BSY       0x80          /* Drive busy — all other bits invalid while set */

/* CF Error Register bits: */
#define ST_ERR_AMNF         0x01          /* Address mark not found */
#define ST_ERR_ABRT         0x04          /* Command aborted */
#define ST_ERR_IDNF         0x10          /* ID not found */
#define ST_ERR_UNC          0x40          /* Uncorrectable data error */
#define ST_ERR_BBK          0x80          /* Bad block detected */

/* ATA Command bytes: */
#define ST_CMD_READ         0x20          /* Read Sectors */
#define ST_CMD_WRITE        0x30          /* Write Sectors */
#define ST_CMD_SET_FEAT     0xEF          /* Set Features */
#define ST_FEAT_8BIT        0x01          /* Feature: enable 8-bit data I/O */
#define ST_LBA3_MASTER      0xE0          /* LBA mode, master drive */

/* Filesystem constants: */
#define FS_DIR_LBA          0             /* Directory sector at LBA 0 */
#define FS_DATA_START       1             /* Data sectors start at LBA 1 */
#define FS_MAX_FILES        16            /* Maximum directory entries */
#define FS_ENTRY_SIZE       32            /* Bytes per directory entry */
#define FS_ENTRY_FLAGS      11            /* Offset: flags byte within entry */
#define FS_ENTRY_START      12            /* Offset: start sector (2 bytes, LE) */
#define FS_ENTRY_FSIZE      14            /* Offset: file size (2 bytes, LE) */
#define FS_FLAG_USED        0x01          /* Flags bit 0: entry in use */
#define FS_DISK_SECTORS     2048          /* Sectors per disk bank (1 MB); 256 banks = 256 MB */

/* XModem protocol constants: */
#define XMODEM_SOH          0x01          /* Start of Header */
#define XMODEM_EOT          0x04          /* End of Transmission */
#define XMODEM_ACK          0x06          /* Acknowledge */
#define XMODEM_NAK          0x15          /* Negative Acknowledge */
#define XMODEM_CAN          0x18          /* Cancel */
#define XMODEM_SUB          0x1A          /* Padding byte */
#define XMODEM_BLKSZ        128           /* Data bytes per block */
#define XMODEM_MAXRETRY     10            /* Max retries per block */
#define XMODEM_STARTRETRY   60            /* Retries waiting for initial connect (~60s) */

/* --- Serial Card (R65C51 ACIA) (IO 5: $9000-$9003) --- */
/* Default config after InitSC: 8-N-1, 19200 baud, RX IRQ enabled */
#define SC_DATA             (*(volatile unsigned char *)0x9000)  /* R/W  — Data register */
#define SC_RESET            (*(volatile unsigned char *)0x9001)  /* Write— Programmatic reset (any value) */
#define SC_STATUS           (*(volatile unsigned char *)0x9001)  /* Read — Status register */
#define SC_CMD              (*(volatile unsigned char *)0x9002)  /* R/W  — Command register */
#define SC_CTRL             (*(volatile unsigned char *)0x9003)  /* R/W  — Control register */

/* Serial Status Register bits: */
#define SC_STATUS_IRQ       0x80          /* Interrupt occurred */
#define SC_STATUS_DSR       0x40          /* Data Set Ready */
#define SC_STATUS_DCD       0x20          /* Data Carrier Detect */
#define SC_STATUS_TDRE      0x10          /* TX Data Register Empty (ready to send) */
#define SC_STATUS_RDRF      0x08          /* RX Data Register Full (byte available) */
#define SC_STATUS_OVR       0x04          /* Overrun error */
#define SC_STATUS_FRME      0x02          /* Framing error */
#define SC_STATUS_PARE      0x01          /* Parity error */

/* Serial Command Register bits: */
#define SC_CMD_RXIRQ_OFF    0x02          /* Receiver interrupt disabled (set = off).
                                           * Set it before polling SC_DATA yourself,
                                           * as XModem does; while it is clear, Chrout
                                           * moves a received byte into the input buffer */

/* --- GPIO / Input Board (65C22 VIA) (IO 6: $9400-$940F) --- */
/* Port B = matrix keyboard / joystick 1, Port A = PS/2 keyboard / joystick 2.
 * CB1/CA1 IRQs fire when a key scancode is ready. */
#define GPIO_PORTB          (*(volatile unsigned char *)0x9400)  /* Port B data */
#define GPIO_PORTA          (*(volatile unsigned char *)0x9401)  /* Port A data */
#define GPIO_DDRB           (*(volatile unsigned char *)0x9402)  /* Port B data direction */
#define GPIO_DDRA           (*(volatile unsigned char *)0x9403)  /* Port A data direction */
#define GPIO_T1CL           (*(volatile unsigned char *)0x9404)  /* Timer 1 counter low */
#define GPIO_T1CH           (*(volatile unsigned char *)0x9405)  /* Timer 1 counter high (write starts timer) */
#define GPIO_T1LL           (*(volatile unsigned char *)0x9406)  /* Timer 1 latch low */
#define GPIO_T1LH           (*(volatile unsigned char *)0x9407)  /* Timer 1 latch high */
#define GPIO_T2CL           (*(volatile unsigned char *)0x9408)  /* Timer 2 counter low */
#define GPIO_T2CH           (*(volatile unsigned char *)0x9409)  /* Timer 2 counter high */
#define GPIO_SR             (*(volatile unsigned char *)0x940A)  /* Shift register */
#define GPIO_ACR            (*(volatile unsigned char *)0x940B)  /* Auxiliary control register */
#define GPIO_PCR            (*(volatile unsigned char *)0x940C)  /* Peripheral control register */
#define GPIO_IFR            (*(volatile unsigned char *)0x940D)  /* Interrupt flag register */
#define GPIO_IER            (*(volatile unsigned char *)0x940E)  /* Interrupt enable register */
#define GPIO_ORA            (*(volatile unsigned char *)0x940F)  /* Port A (no handshake) */

/* VIA Peripheral Control Register (PCR) mode bits: */
#define GPIO_PCR_CB2_LO     0xC0          /* CB2 output low  (enable matrix keyboard encoder) */
#define GPIO_PCR_CB2_HI     0xE0          /* CB2 output high (disable matrix keyboard encoder) */
#define GPIO_PCR_CB1_NEG    0x00          /* CB1 active on falling edge */
#define GPIO_PCR_CA2_LO     0x0C          /* CA2 output low  (enable PS/2 keyboard encoder) */
#define GPIO_PCR_CA2_HI     0x0E          /* CA2 output high (disable PS/2 keyboard encoder) */
#define GPIO_PCR_CA1_NEG    0x00          /* CA1 active on falling edge */

/* VIA Interrupt flag/enable bits: */
#define GPIO_INT_CB1        0x10          /* Bit 4: CB1 interrupt (matrix keyboard data ready) */
#define GPIO_INT_CA1        0x02          /* Bit 1: CA1 interrupt (PS/2 keyboard data ready) */
#define GPIO_IER_SET        0x80          /* IER bit 7: 1 = set named bits, 0 = clear them */

/* --- Sound Card (SID / ARMSID) (IO 7: $9800-$981C) --- */
/* MOS 6581/8580 compatible.  3 voices, each with independent frequency, pulse
 * width, waveform and ADSR envelope.  Global filter and volume. */

/* Voice 1: */
#define SID_V1_FREQ_LO      (*(volatile unsigned char *)0x9800)  /* Frequency low byte */
#define SID_V1_FREQ_HI      (*(volatile unsigned char *)0x9801)  /* Frequency high byte */
#define SID_V1_PW_LO        (*(volatile unsigned char *)0x9802)  /* Pulse width low byte */
#define SID_V1_PW_HI        (*(volatile unsigned char *)0x9803)  /* Pulse width high nibble */
#define SID_V1_CTRL         (*(volatile unsigned char *)0x9804)  /* Control register (waveform + gate) */
#define SID_V1_AD           (*(volatile unsigned char *)0x9805)  /* Attack / Decay */
#define SID_V1_SR           (*(volatile unsigned char *)0x9806)  /* Sustain / Release */

/* Voice 2: */
#define SID_V2_FREQ_LO      (*(volatile unsigned char *)0x9807)
#define SID_V2_FREQ_HI      (*(volatile unsigned char *)0x9808)
#define SID_V2_PW_LO        (*(volatile unsigned char *)0x9809)
#define SID_V2_PW_HI        (*(volatile unsigned char *)0x980A)
#define SID_V2_CTRL         (*(volatile unsigned char *)0x980B)
#define SID_V2_AD           (*(volatile unsigned char *)0x980C)
#define SID_V2_SR           (*(volatile unsigned char *)0x980D)

/* Voice 3: */
#define SID_V3_FREQ_LO      (*(volatile unsigned char *)0x980E)
#define SID_V3_FREQ_HI      (*(volatile unsigned char *)0x980F)
#define SID_V3_PW_LO        (*(volatile unsigned char *)0x9810)
#define SID_V3_PW_HI        (*(volatile unsigned char *)0x9811)
#define SID_V3_CTRL         (*(volatile unsigned char *)0x9812)
#define SID_V3_AD           (*(volatile unsigned char *)0x9813)
#define SID_V3_SR           (*(volatile unsigned char *)0x9814)

/* Global SID registers: */
#define SID_FC_LO           (*(volatile unsigned char *)0x9815)  /* Filter cutoff low (bits 0-2) */
#define SID_FC_HI           (*(volatile unsigned char *)0x9816)  /* Filter cutoff high */
#define SID_RES_FILT        (*(volatile unsigned char *)0x9817)  /* Resonance / filter enable per voice */
#define SID_MODE_VOL        (*(volatile unsigned char *)0x9818)  /* Filter mode / master volume (low nibble) */
#define SID_PADDLE_X        (*(volatile unsigned char *)0x9819)  /* Paddle X (read-only) */
#define SID_PADDLE_Y        (*(volatile unsigned char *)0x981A)  /* Paddle Y (read-only) */
#define SID_OSC3            (*(volatile unsigned char *)0x981B)  /* Oscillator 3 output (read-only) */
#define SID_ENV3            (*(volatile unsigned char *)0x981C)  /* Envelope 3 output (read-only) */

/* SID Voice Control Register bits (apply to SID_Vn_CTRL): */
#define SID_CTRL_GATE       0x01          /* Gate: 1 = start attack, 0 = start release */
#define SID_CTRL_SYNC       0x02          /* Hard sync with preceding voice */
#define SID_CTRL_RING       0x04          /* Ring modulation with preceding voice */
#define SID_CTRL_TEST       0x08          /* Test bit (resets oscillator) */
#define SID_CTRL_TRIANGLE   0x10          /* Triangle waveform */
#define SID_CTRL_SAWTOOTH   0x20          /* Sawtooth waveform */
#define SID_CTRL_PULSE      0x40          /* Pulse waveform */
#define SID_CTRL_NOISE      0x80          /* Noise waveform */

/* SID Filter Mode bits (high nibble of SID_MODE_VOL): */
#define SID_FILT_LP         0x10          /* Low-pass filter */
#define SID_FILT_BP         0x20          /* Band-pass filter */
#define SID_FILT_HP         0x40          /* High-pass filter */
#define SID_FILT_3OFF       0x80          /* Disconnect voice 3 from output */

/* --- SID Frequency Table (1 MHz clock, equal temperament) --- */
/* Freq register = round(Hz x 16777216 / 1000000).  Octaves 2-7, all 12
 * chromatic notes; sharps and flats are aliases.  Pass one to SidPlayNote:
 *
 *     SidPlayNote(0, SID_NOTE_A4);
 */

/* Octave 2 (C2=65.41 Hz .. B2=123.47 Hz) */
#define SID_NOTE_C2          0x0449                /* ~65 Hz */
#define SID_NOTE_CS2         0x048B                /* ~69 Hz */
#define SID_NOTE_Db2         SID_NOTE_CS2
#define SID_NOTE_D2          0x04D0                /* ~73 Hz */
#define SID_NOTE_DS2         0x0519                /* ~78 Hz */
#define SID_NOTE_Eb2         SID_NOTE_DS2
#define SID_NOTE_E2          0x0567                /* ~82 Hz */
#define SID_NOTE_F2          0x05B9                /* ~87 Hz */
#define SID_NOTE_FS2         0x0610                /* ~93 Hz */
#define SID_NOTE_Gb2         SID_NOTE_FS2
#define SID_NOTE_G2          0x066C                /* ~98 Hz */
#define SID_NOTE_GS2         0x06CE                /* ~104 Hz */
#define SID_NOTE_Ab2         SID_NOTE_GS2
#define SID_NOTE_A2          0x0735                /* ~110 Hz */
#define SID_NOTE_AS2         0x07A3                /* ~117 Hz */
#define SID_NOTE_Bb2         SID_NOTE_AS2
#define SID_NOTE_B2          0x0817                /* ~123 Hz */

/* Octave 3 (C3=130.81 Hz .. B3=246.94 Hz) */
#define SID_NOTE_C3          0x0893                /* ~131 Hz */
#define SID_NOTE_CS3         0x0915                /* ~139 Hz */
#define SID_NOTE_Db3         SID_NOTE_CS3
#define SID_NOTE_D3          0x099F                /* ~147 Hz */
#define SID_NOTE_DS3         0x0A32                /* ~156 Hz */
#define SID_NOTE_Eb3         SID_NOTE_DS3
#define SID_NOTE_E3          0x0ACD                /* ~165 Hz */
#define SID_NOTE_F3          0x0B72                /* ~175 Hz */
#define SID_NOTE_FS3         0x0C20                /* ~185 Hz */
#define SID_NOTE_Gb3         SID_NOTE_FS3
#define SID_NOTE_G3          0x0CD8                /* ~196 Hz */
#define SID_NOTE_GS3         0x0D9C                /* ~208 Hz */
#define SID_NOTE_Ab3         SID_NOTE_GS3
#define SID_NOTE_A3          0x0E6B                /* ~220 Hz */
#define SID_NOTE_AS3         0x0F46                /* ~233 Hz */
#define SID_NOTE_Bb3         SID_NOTE_AS3
#define SID_NOTE_B3          0x102F                /* ~247 Hz */

/* Octave 4 (C4=261.63 Hz .. B4=493.88 Hz) — middle octave */
#define SID_NOTE_C4          0x1125                /* ~262 Hz (Middle C) */
#define SID_NOTE_CS4         0x122A                /* ~277 Hz */
#define SID_NOTE_Db4         SID_NOTE_CS4
#define SID_NOTE_D4          0x133F                /* ~294 Hz */
#define SID_NOTE_DS4         0x1464                /* ~311 Hz */
#define SID_NOTE_Eb4         SID_NOTE_DS4
#define SID_NOTE_E4          0x159A                /* ~330 Hz */
#define SID_NOTE_F4          0x16E3                /* ~349 Hz */
#define SID_NOTE_FS4         0x183F                /* ~370 Hz */
#define SID_NOTE_Gb4         SID_NOTE_FS4
#define SID_NOTE_G4          0x19B1                /* ~392 Hz */
#define SID_NOTE_GS4         0x1B38                /* ~415 Hz */
#define SID_NOTE_Ab4         SID_NOTE_GS4
#define SID_NOTE_A4          0x1CD6                /* ~440 Hz (Concert A) */
#define SID_NOTE_AS4         0x1E8D                /* ~466 Hz */
#define SID_NOTE_Bb4         SID_NOTE_AS4
#define SID_NOTE_B4          0x205E                /* ~494 Hz */

/* Octave 5 (C5=523.25 Hz .. B5=987.77 Hz) */
#define SID_NOTE_C5          0x224B                /* ~523 Hz */
#define SID_NOTE_CS5         0x2455                /* ~554 Hz */
#define SID_NOTE_Db5         SID_NOTE_CS5
#define SID_NOTE_D5          0x267E                /* ~587 Hz */
#define SID_NOTE_DS5         0x28C8                /* ~622 Hz */
#define SID_NOTE_Eb5         SID_NOTE_DS5
#define SID_NOTE_E5          0x2B34                /* ~659 Hz */
#define SID_NOTE_F5          0x2DC6                /* ~698 Hz */
#define SID_NOTE_FS5         0x307F                /* ~740 Hz */
#define SID_NOTE_Gb5         SID_NOTE_FS5
#define SID_NOTE_G5          0x3361                /* ~784 Hz */
#define SID_NOTE_GS5         0x366F                /* ~831 Hz */
#define SID_NOTE_Ab5         SID_NOTE_GS5
#define SID_NOTE_A5          0x39AC                /* ~880 Hz */
#define SID_NOTE_AS5         0x3D1A                /* ~932 Hz */
#define SID_NOTE_Bb5         SID_NOTE_AS5
#define SID_NOTE_B5          0x40BC                /* ~988 Hz */

/* Octave 6 (C6=1046.50 Hz .. B6=1975.53 Hz) */
#define SID_NOTE_C6          0x4495                /* ~1047 Hz */
#define SID_NOTE_CS6         0x48A9                /* ~1109 Hz */
#define SID_NOTE_Db6         SID_NOTE_CS6
#define SID_NOTE_D6          0x4CFC                /* ~1175 Hz */
#define SID_NOTE_DS6         0x518F                /* ~1245 Hz */
#define SID_NOTE_Eb6         SID_NOTE_DS6
#define SID_NOTE_E6          0x5669                /* ~1319 Hz */
#define SID_NOTE_F6          0x5B8C                /* ~1397 Hz */
#define SID_NOTE_FS6         0x60FE                /* ~1480 Hz */
#define SID_NOTE_Gb6         SID_NOTE_FS6
#define SID_NOTE_G6          0x66C2                /* ~1568 Hz */
#define SID_NOTE_GS6         0x6CDF                /* ~1661 Hz */
#define SID_NOTE_Ab6         SID_NOTE_GS6
#define SID_NOTE_A6          0x7358                /* ~1760 Hz */
#define SID_NOTE_AS6         0x7A34                /* ~1865 Hz */
#define SID_NOTE_Bb6         SID_NOTE_AS6
#define SID_NOTE_B6          0x8178                /* ~1976 Hz */

/* Octave 7 (C7=2093.00 Hz .. A7=3520.00 Hz; B7 overflows 16-bit register) */
#define SID_NOTE_C7          0x892B                /* ~2093 Hz */
#define SID_NOTE_CS7         0x9153                /* ~2217 Hz */
#define SID_NOTE_Db7         SID_NOTE_CS7
#define SID_NOTE_D7          0x99F7                /* ~2349 Hz */
#define SID_NOTE_DS7         0xA31F                /* ~2489 Hz */
#define SID_NOTE_Eb7         SID_NOTE_DS7
#define SID_NOTE_E7          0xACD2                /* ~2637 Hz */
#define SID_NOTE_F7          0xB719                /* ~2794 Hz */
#define SID_NOTE_FS7         0xC1FC                /* ~2960 Hz */
#define SID_NOTE_Gb7         SID_NOTE_FS7
#define SID_NOTE_G7          0xCD85                /* ~3136 Hz */
#define SID_NOTE_GS7         0xD9BD                /* ~3322 Hz */
#define SID_NOTE_Ab7         SID_NOTE_GS7
#define SID_NOTE_A7          0xE6B0                /* ~3520 Hz */
#define SID_NOTE_AS7         0xF467                /* ~3729 Hz */
#define SID_NOTE_Bb7         SID_NOTE_AS7
/* B7 (~3951 Hz = 0x102F0) exceeds the 16-bit SID register — not defined. */

/* --- Video Card (TMS9918 / pico9918) (IO 8: $9C00-$9C01) --- */
/* Text mode: 40 columns x 24 rows.  Name table at VRAM $0000 (960 bytes),
 * pattern table at VRAM $0800 (2 KB, 256 x 8-byte patterns).
 * To write a VRAM address: low byte to VC_REG, then (high | 0x40) to VC_REG.
 * To read one: low byte to VC_REG, then high byte to VC_REG.
 * Register writes: data byte to VC_REG, then (reg# | 0x80) to VC_REG. */
#define VC_DATA             (*(volatile unsigned char *)0x9C00)  /* R/W  — VRAM data (auto-increments) */
#define VC_REG              (*(volatile unsigned char *)0x9C01)  /* Write— Address/register port */
#define VC_STATUS           (*(volatile unsigned char *)0x9C01)  /* Read — Status register */


/* =============================================================================
 *   SECTION 5b: IO REGISTER BLOCKS
 * =============================================================================
 * The registers of Section 5 again, reached as one struct per card instead of
 * one macro per address.  C only — 6502.inc has no counterpart, because
 * assembly already addresses the registers directly.
 *
 * Each block is a struct laid out to match the card's register map, overlaid
 * on that card's base address:
 *
 *     SID.voice[0].freq = SID_NOTE_A4;    ~ same two bytes as SID_V1_FREQ_LO/HI
 *     GPIO.pcr          = GPIO_PCR_CB2_LO;
 *     if (ST.status & ST_STATUS_BSY) { ... }
 *
 * Nothing is gained or lost at run time.  A field access with a constant index
 * compiles to the same absolute load or store the Section 5 macro does —
 * SID.voice[1].ctrl = x is one `sta $980B`, not pointer arithmetic.  A
 * *variable* index is the exception: SID.voice[i] has to multiply and build a
 * pointer, so keep the index constant in code that runs often, and use the
 * array where looping is the point.
 *
 * What the blocks add over the flat macros:
 *
 *   - Indexing.  SID.voice[i], ST.lba[i], RAM_L.data[i] — the flat names
 *     cannot be indexed, so a loop over three voices has to be unrolled.
 *   - 16-bit pairs.  SID.voice[0].freq writes low byte then high byte in one
 *     statement, which is the order the SID and the VIA timers both want, and
 *     it takes the SID_NOTE_* constants directly.
 *   - Grouping.  Every register of one card under one name, so typing "GPIO."
 *     lists the VIA's registers in the editor's completion popup.
 *
 * Registers that mean different things read and written (the CF status/command
 * port, the ACIA status/reset port, the VDP register/status port) appear once
 * under each name, in a union.
 *
 * Every block is volatile, so each field access is a real bus cycle that the
 * optimizer will not fold away, reorder or keep in a register.  The Section 5
 * macros are unchanged and remain the documented spelling; these are a second
 * view of the same addresses, and the two mix freely.
 */

/* --- Banked RAM Low / High (IO 1: $8000-$83FF, IO 2: $8400-$87FF) --- */
/* Write the bank number to the latch, then use the window:
 *     RAM_L.bank = 7;  RAM_L.data[0] = 0x42; */
struct __ac_ram {
    unsigned char       data[1023];     /* +$000  R/W data window */
    unsigned char       bank;           /* +$3FF  Write-only bank latch (0x00-0xFF) */
};

#define RAM_L               (*(volatile struct __ac_ram *)0x8000)
#define RAM_H               (*(volatile struct __ac_ram *)0x8400)

/* --- RTC (DS1511Y) (IO 3: $8800-$881F) --- */
/* Clock fields are BCD.  The Kernal's RtcReadTime / RtcWriteTime convert to
 * and from binary; these are the raw registers.  Mask the month with
 * RTC_MON_MASK — its top 3 bits are oscillator control, not month data. */
struct __ac_rtc {
    unsigned char       sec;            /* +$00  Seconds      (BCD, 00-59) */
    unsigned char       min;            /* +$01  Minutes      (BCD, 00-59) */
    unsigned char       hr;             /* +$02  Hours        (BCD, 00-23, 24h mode) */
    unsigned char       day;            /* +$03  Day of week  (1-7) */
    unsigned char       date;           /* +$04  Day of month (BCD, 01-31) */
    unsigned char       mon;            /* +$05  Month        (BCD, 01-12) + osc control */
    unsigned char       yr;             /* +$06  Year         (BCD, 00-99) */
    unsigned char       cent;           /* +$07  Century      (BCD, 00-39) */
    unsigned char       alarm_sec;      /* +$08  Alarm seconds */
    unsigned char       alarm_min;      /* +$09  Alarm minutes */
    unsigned char       alarm_hr;       /* +$0A  Alarm hours */
    unsigned char       alarm_date;     /* +$0B  Alarm day of month */
    unsigned char       wd_l;           /* +$0C  Watchdog low */
    unsigned char       wd_h;           /* +$0D  Watchdog high */
    unsigned char       ctrl_a;         /* +$0E  Control Register A */
    unsigned char       ctrl_b;         /* +$0F  Control Register B */
    unsigned char       ram_addr;       /* +$10  NVRAM address register */
    unsigned char       pad[2];         /* +$11  (not decoded) */
    unsigned char       ram_data;       /* +$13  NVRAM data register */
};

#define RTC                 (*(volatile struct __ac_rtc *)0x8800)

/* --- CompactFlash / Storage (IO 4: $8C00-$8C07) --- */
/* 8-bit IDE (True IDE) mode.  Two ports read and write differently:
 * +$01 is error/feature, +$07 is status/command. */
struct __ac_st {
    unsigned char       data;           /* +$00  R/W  Data register */
    union {
        unsigned char   error;          /* +$01  Read  Error register */
        unsigned char   feature;        /* +$01  Write Feature register */
    };
    unsigned char       sect_cnt;       /* +$02  R/W  Sector count */
    union {
        struct {
            unsigned char lba_0;        /* +$03  R/W  LBA bits 0-7 */
            unsigned char lba_1;        /* +$04  R/W  LBA bits 8-15 */
            unsigned char lba_2;        /* +$05  R/W  LBA bits 16-23 */
            unsigned char lba_3;        /* +$06  R/W  LBA bits 24-27 + drive/mode */
        };
        unsigned char   lba[4];         /* the same four, for a loop */
    };
    union {
        unsigned char   status;         /* +$07  Read  Status register */
        unsigned char   cmd;            /* +$07  Write Command register */
    };
};

#define ST                  (*(volatile struct __ac_st *)0x8C00)

/* --- Serial Card (R65C51 ACIA) (IO 5: $9000-$9003) --- */
/* +$01 reads as status and writes as a programmatic reset (any value). */
struct __ac_sc {
    unsigned char       data;           /* +$00  R/W  Data register */
    union {
        unsigned char   status;         /* +$01  Read  Status register */
        unsigned char   reset;          /* +$01  Write Programmatic reset */
    };
    unsigned char       cmd;            /* +$02  R/W  Command register */
    unsigned char       ctrl;           /* +$03  R/W  Control register */
};

#define SC                  (*(volatile struct __ac_sc *)0x9000)

/* --- GPIO / Input Board (65C22 VIA) (IO 6: $9400-$940F) --- */
/* Port B = matrix keyboard / joystick 1, Port A = PS/2 keyboard / joystick 2.
 * The 16-bit timer names write low byte then high byte, which is the order the
 * VIA wants — writing T1C-H is what starts timer 1. */
struct __ac_gpio {
    unsigned char       portb;          /* +$00  Port B data */
    unsigned char       porta;          /* +$01  Port A data */
    unsigned char       ddrb;           /* +$02  Port B data direction */
    unsigned char       ddra;           /* +$03  Port A data direction */
    union {
        unsigned int    t1c;            /* +$04  Timer 1 counter, 16-bit */
        struct {
            unsigned char t1cl;         /* +$04  Timer 1 counter low */
            unsigned char t1ch;         /* +$05  Timer 1 counter high (write starts timer) */
        };
    };
    union {
        unsigned int    t1l;            /* +$06  Timer 1 latch, 16-bit */
        struct {
            unsigned char t1ll;         /* +$06  Timer 1 latch low */
            unsigned char t1lh;         /* +$07  Timer 1 latch high */
        };
    };
    union {
        unsigned int    t2c;            /* +$08  Timer 2 counter, 16-bit */
        struct {
            unsigned char t2cl;         /* +$08  Timer 2 counter low */
            unsigned char t2ch;         /* +$09  Timer 2 counter high */
        };
    };
    unsigned char       sr;             /* +$0A  Shift register */
    unsigned char       acr;            /* +$0B  Auxiliary control register */
    unsigned char       pcr;            /* +$0C  Peripheral control register */
    unsigned char       ifr;            /* +$0D  Interrupt flag register */
    unsigned char       ier;            /* +$0E  Interrupt enable register */
    unsigned char       ora;            /* +$0F  Port A (no handshake) */
};

#define GPIO                (*(volatile struct __ac_gpio *)0x9400)

/* --- Sound Card (SID / ARMSID) (IO 7: $9800-$981C) --- */
/* Three identical 7-byte voices followed by the global registers.  The voices
 * are reachable both by name (SID.v1) and by index (SID.voice[0]), and `freq`
 * takes a SID_NOTE_* constant whole:
 *
 *     SID.voice[0].freq = SID_NOTE_A4;
 *     SID.voice[0].ad   = 0x09;
 *     SID.voice[0].ctrl = SID_CTRL_TRIANGLE | SID_CTRL_GATE; */
struct __ac_sid_voice {
    union {
        unsigned int    freq;           /* +$00  Frequency, 16-bit (SID_NOTE_*) */
        struct {
            unsigned char freq_lo;      /* +$00  Frequency low byte */
            unsigned char freq_hi;      /* +$01  Frequency high byte */
        };
    };
    union {
        unsigned int    pw;             /* +$02  Pulse width, 12-bit in 16 */
        struct {
            unsigned char pw_lo;        /* +$02  Pulse width low byte */
            unsigned char pw_hi;        /* +$03  Pulse width high nibble */
        };
    };
    unsigned char       ctrl;           /* +$04  Control register (waveform + gate) */
    unsigned char       ad;             /* +$05  Attack / Decay */
    unsigned char       sr;             /* +$06  Sustain / Release */
};

struct __ac_sid {
    union {
        struct {
            struct __ac_sid_voice v1;   /* +$00  Voice 1 */
            struct __ac_sid_voice v2;   /* +$07  Voice 2 */
            struct __ac_sid_voice v3;   /* +$0E  Voice 3 */
        };
        struct __ac_sid_voice voice[3]; /* the same three, for a loop */
    };
    union {
        unsigned int    fc;             /* +$15  Filter cutoff, 11-bit in 16 */
        struct {
            unsigned char fc_lo;        /* +$15  Filter cutoff low (bits 0-2) */
            unsigned char fc_hi;        /* +$16  Filter cutoff high */
        };
    };
    unsigned char       res_filt;       /* +$17  Resonance / filter enable per voice */
    unsigned char       mode_vol;       /* +$18  Filter mode / master volume (low nibble) */
    unsigned char       paddle_x;       /* +$19  Paddle X (read-only) */
    unsigned char       paddle_y;       /* +$1A  Paddle Y (read-only) */
    unsigned char       osc3;           /* +$1B  Oscillator 3 output (read-only) */
    unsigned char       env3;           /* +$1C  Envelope 3 output (read-only) */
};

#define SID                 (*(volatile struct __ac_sid *)0x9800)

/* --- Video Card (TMS9918 / pico9918) (IO 8: $9C00-$9C01) --- */
/* Two ports, and +$01 reads as status but writes as the address/register port.
 * A VDP register write is two bytes to that one port — the value, then the
 * register number with VC_REG_WRITE set:
 *
 *     VC.reg = (TMS_LT_GREEN << 4) | TMS_BLACK;
 *     VC.reg = VC_REG_WRITE | VC_REG_COLOR;
 *
 * which is what the Kernal's VideoSetColor() does in one call.  Setting a VRAM
 * address is the same shape: low byte, then high byte ORed with VC_ADDR_WRITE
 * to write or left bare to read, after which VC.data auto-increments. */
struct __ac_vc {
    unsigned char       data;           /* +$00  R/W  VRAM data (auto-increments) */
    union {
        unsigned char   reg;            /* +$01  Write Address/register port */
        unsigned char   status;         /* +$01  Read  Status register */
    };
};

#define VC                  (*(volatile struct __ac_vc *)0x9C00)

#define VC_REG_WRITE        0x80          /* OR with a register number to select it */
#define VC_ADDR_WRITE       0x40          /* OR with a VRAM address high byte to write */
#define VC_REG_COLOR        7             /* Register 7: (text colour << 4) | backdrop */


/* =============================================================================
 *   SECTION 6: USEFUL CONSTANTS
 * =============================================================================
 */

/* --- BIOS Version --- */
#define BIOS_VERSION_MAJOR  1
#define BIOS_VERSION_MINOR  6

/* --- NVRAM Save Slots (v1.6) --- */
#define NV_SLOTS            16            /* Save slots in RTC NVRAM */
#define NV_SLOT_SIZE        16            /* Bytes per slot (slot n at NVRAM n*16) */
#define NV_SLOT_DATA        14            /* Payload bytes per slot */
#define NV_CK_SEED          0xA6          /* Checksum seed */
#define NV_EMPTY            0             /* NvStat: slot is free */
#define NV_VALID            1             /* NvStat: owner ID set, checksum agrees */
#define NV_BAD              2             /* NvStat: owner ID set, checksum does not */
#define NV_ERROR            0xFF          /* NvStat / NvRead: no RTC, or slot not 0-15 */
#define NV_NONE             0xFF          /* NvFind: no slot matched */

/* --- ASCII Control Characters --- */
#define CHAR_BEL            0x07          /* Bell (beep via SID) */
#define CHAR_BS             0x08          /* Backspace (cursor left + erase in video mode) */
#define CHAR_LF             0x0A          /* Line Feed (cursor down, scroll if at bottom) */
#define CHAR_CR             0x0D          /* Carriage Return (cursor to column 0) */
#define CHAR_ESC            0x1B          /* Escape */
#define CHAR_SPACE          0x20          /* Space */

/* --- Video Dimensions --- */
#define VID_COLS            40            /* Screen width in columns */
#define VID_ROWS            24            /* Screen height in rows */
#define VID_NAME_TABLE      0x0000        /* VRAM address of name table (40x24 = 960 bytes) */
#define VID_PATTERN_TABLE   0x0800        /* VRAM address of pattern table (2048 bytes) */

/* --- TMS9918 Color Constants (for VideoSetColor) --- */
/* Pass as (foreground << 4) | background.
 * Example: white on black = 0xF0, green on black = 0x20 */
#define TMS_TRANSPARENT     0x0
#define TMS_BLACK           0x1
#define TMS_MED_GREEN       0x2
#define TMS_LT_GREEN        0x3
#define TMS_DK_BLUE         0x4
#define TMS_LT_BLUE         0x5
#define TMS_DK_RED          0x6
#define TMS_CYAN            0x7
#define TMS_MED_RED         0x8
#define TMS_LT_RED          0x9
#define TMS_DK_YELLOW       0xA
#define TMS_LT_YELLOW       0xB
#define TMS_DK_GREEN        0xC
#define TMS_MAGENTA         0xD
#define TMS_GRAY            0xE
#define TMS_WHITE           0xF

/* --- Monitor Entry Points --- */
/* The machine code monitor lives at $EE00 and is normally entered via BRK.
 * These are useful if you want to jump there directly. */
#define MONITOR_ENTRY       ((void (*)(void))0xEE00)   /* Cold entry */
#define MONITOR_BRK_ENTRY   ((void (*)(void))0xEE03)   /* BRK entry (displays saved registers) */


#endif  /* _AC6502_H */
