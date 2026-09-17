; =============================================================================
;   fs.s — Filesystem and CompactFlash storage
; =============================================================================

        .setcpu "65C02"

        .export _FsLoadFile, _FsSaveFile, _FsDeleteFile
        .export _FsLoadFileAddr, _FsSaveFileAddr
        .export _FsFormatDisk, _FsSetDisk, _FsGetDisk, _FsPrintDisk
        .export _StWaitReady

        .import carryerr
        .import popax

        .ifdef VDP
        .include "6502-VDP.inc"
        .else
        .include "6502.inc"
        .endif

.code

; unsigned char __fastcall__ FsLoadFile(const char *name);
_FsLoadFile:
        jsr     SetName
        jsr     FsLoadFile
        jmp     carryerr

; unsigned char __fastcall__ FsSaveFile(const char *name);
_FsSaveFile:
        jsr     SetName
        jsr     FsSaveFile
        jmp     carryerr

; unsigned char __fastcall__ FsDeleteFile(const char *name);
_FsDeleteFile:
        jsr     SetName
        jsr     FsDeleteFile
        jmp     carryerr

; unsigned char __fastcall__ FsLoadFileAddr(const char *name, void *addr);
;   addr arrives in A/X, name is on the C stack.
_FsLoadFileAddr:
        sta     FS_IO_ADDR
        stx     FS_IO_ADDR+1
        jsr     popax           ; A/X = name
        jsr     SetName
        jsr     FsLoadFileAddr
        jmp     carryerr

; unsigned char __fastcall__ FsSaveFileAddr(const char *name,
;                                            const void *addr,
;                                            unsigned int size);
;   size arrives in A/X; addr and then name come off the C stack in reverse.
_FsSaveFileAddr:
        sta     FS_FILE_SIZE
        stx     FS_FILE_SIZE+1
        jsr     popax           ; A/X = addr
        sta     FS_IO_ADDR
        stx     FS_IO_ADDR+1
        jsr     popax           ; A/X = name
        jsr     SetName
        jsr     FsSaveFileAddr
        jmp     carryerr

; unsigned char FsFormatDisk(void);
_FsFormatDisk:
        jsr     FsFormatDisk
        jmp     carryerr

; void __fastcall__ FsSetDisk(unsigned char disk);
_FsSetDisk:
        jmp     FsSetDisk

; unsigned char FsGetDisk(void);
_FsGetDisk:
        jsr     FsGetDisk
        ldx     #$00
        rts

; void FsPrintDisk(void);
_FsPrintDisk:
        jmp     FsPrintDisk

; unsigned char StWaitReady(void);
_StWaitReady:
        jsr     StWaitReady
        jmp     carryerr

; -----------------------------------------------------------------------------
;   SetName — park a filename pointer where the FS routines look for it
; -----------------------------------------------------------------------------
;   In: A/X = pointer to a NUL-terminated 8.3 filename

SetName:
        sta     STR_PTR
        stx     STR_PTR+1
        rts
