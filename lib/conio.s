; =============================================================================
;   conio.s — cc65's console interface on the AC6502
; =============================================================================
;   cc65 splits <conio.h> in two.  The composition half — cputs, cprintf,
;   cputhex, cgets, cscanf — is machine independent and already sits in
;   none.lib; the primitive half is per machine and is what this file is.
;   Supply these and the rest of the header links and works.
;
;   Almost every one is a Kernal call the BIOS already provides:
;
;       clrscr   -> VideoClear          cputc    -> Chrout
;       gotoxy   -> VideoSetCursor      cgetc    -> Chrin, spun on
;       wherex/y -> VID_CURSOR_X/Y      kbhit    -> BufferSize
;
;   Output goes through Chrout, not straight to the video card, so conio
;   follows IO_MODE like everything else — a program using cputs() over the
;   serial console works without knowing it is on serial.
;
;   Placement does not travel that way.  gotoxy, wherex and wherey are the
;   BIOS's video cursor, and VideoSetCursor does nothing at all when no video
;   card is fitted, so on a serial-only machine the text still appears but
;   every position reads back as 0,0.  Write for the screen if you are going
;   to move the cursor; a program that only puts characters in order works
;   either way.
;
;   cputs() in none.lib relies on _cputc not touching ptr1.  That holds here
;   because of how the zero page is split: cc65's runtime starts at $3A (see
;   6502.cfg) and the Kernal keeps to the bytes below it, so no ROM routine
;   can reach ptr1 at $42.
;
;   WHERE THE TMS9918 DOES NOT FIT
;   ------------------------------
;   conio was designed around the VIC-II, which colours every character cell
;   individually.  A TMS9918 in text mode has one foreground and one backdrop
;   for the whole screen, so:
;
;     - textcolor() and bgcolor() recolour the entire screen, not the text
;       written after the call.  They return the previous value and behave
;       sensibly; they just are not per-character.
;     - bordercolor() is the same register as bgcolor() — in text mode the
;       backdrop is the border.
;     - revers() has nothing to drive it.  There is no reverse attribute and
;       no inverse half to the character set, so it records the flag, returns
;       the old one, and changes nothing on screen.
;     - Register 7 cannot be read back, so the colour shadow starts at the
;       value InitVideo leaves there ($1F, black on white).  A program that
;       changes the colour by any other route — VideoSetColor(), or writing
;       register 7 through VC — puts the shadow out of step, and the next
;       textcolor() or bgcolor() will restore the half it thinks it knows.
;
;   ON THE 6502-PICOVDP (the VDP build, BIOS 2.x)
;   ----------------------------------------------
;   The Text console colours every cell, which is what conio expects:
;
;     - textcolor() and bgcolor() set the pen, VID_PEN, so they colour the
;       text written after the call and leave the screen as it is.  VID_PEN
;       is RAM the Kernal keeps, so there is no shadow to fall out of step.
;     - bordercolor() is still bgcolor(): VideoSetColor writes register 7
;       with the pen, and its low nibble, the border, follows the background.
;     - revers() is unchanged: recorded and returned, nothing on screen.
;
;   cursor() links from none.lib and sets a variable nothing here reads; the
;   BIOS owns the cursor.
; =============================================================================

        .setcpu "65C02"

        .export _cputc, cputdirect
        .export gotoxy, _gotoxy, _gotox, _gotoy
        .export _wherex, _wherey
        .export _clrscr
        .export _cgetc, _kbhit
        .export screensize
        .export _cputcxy
        .export _cclear, _cclearxy
        .export _chline, _chlinexy
        .export _cvline, _cvlinexy
        .export _revers
        .export _textcolor, _bgcolor, _bordercolor

        .import popa

        .include "zeropage.inc"
        .ifdef VDP
        .include "6502-VDP.inc"
        .else
        .include "6502.inc"
        .endif

CH_HLINE        = '-'                   ; no line-drawing glyphs in the
CH_VLINE        = '|'                   ;   BIOS character set

.code

; ---------------------------------------------------------------------------
; void __fastcall__ cputc (char c);
;   '\n' is a newline in C and CR+LF on the console, the same expansion
;   write() does for printf.

;   Both halves go through Chrout, which is the routine that acts on a control
;   code.  Falling through to cputdirect instead would draw the LF as a glyph
;   and leave the cursor one column along on the same row.

_cputc:
        cmp     #$0A                    ; '\n'?
        bne     cputdirect
        lda     #CHAR_CR
        jsr     Chrout                  ; CR first...
        lda     #CHAR_LF
        jmp     Chrout                  ; ...then the LF, also interpreted

; ---------------------------------------------------------------------------
; cputdirect — one glyph, no control-code handling.
;   VideoChroutRaw is exactly this, but only exists for the video card, so
;   serial falls back to Chrout.

cputdirect:
        pha
        lda     IO_MODE
        lsr     a                       ; bit 0 into carry: 1 = serial
        pla
        bcs     @serial
        jmp     VideoChroutRaw
@serial:
        jmp     Chrout

; ---------------------------------------------------------------------------
; void __fastcall__ gotoxy (unsigned char x, unsigned char y);
;   gotoxy takes both off the C stack (cputsxy and friends call it that way);
;   _gotoxy is the C entry, with y in A and x still on the stack.

gotoxy:
        jsr     popa                    ; A = y

_gotoxy:
        sta     tmp1                    ; row
        jsr     popa                    ; A = x
        tax                             ; X = col
        ldy     tmp1                    ; Y = row
        jmp     VideoSetCursor

; void __fastcall__ gotox (unsigned char x);
_gotox:
        tax                             ; X = col
        ldy     VID_CURSOR_Y            ; keep the row
        jmp     VideoSetCursor

; void __fastcall__ gotoy (unsigned char y);
_gotoy:
        tay                             ; Y = row
        ldx     VID_CURSOR_X            ; keep the column
        jmp     VideoSetCursor

; ---------------------------------------------------------------------------
; unsigned char wherex (void);
; unsigned char wherey (void);
;   The BIOS keeps the cursor in two system variables, so these are reads.

_wherex:
        lda     VID_CURSOR_X
        ldx     #$00
        rts

_wherey:
        lda     VID_CURSOR_Y
        ldx     #$00
        rts

; ---------------------------------------------------------------------------
; void clrscr (void);

_clrscr:
        jmp     VideoClear

; ---------------------------------------------------------------------------
; char cgetc (void);
;   Chrin is a poll, so spin until it yields a character.

_cgetc:
        jsr     Chrin
        bcc     _cgetc
        ldx     #$00
        rts

; unsigned char kbhit (void);
;   Anything waiting in the input buffer counts, so collapse the count to 0/1.

_kbhit:
        jsr     BufferSize              ; A = unread bytes
        cmp     #$01                    ; carry set if at least one
        lda     #$00
        ldx     #$00
        rol     a                       ; A = carry
        rts

; ---------------------------------------------------------------------------
; screensize — X = width, Y = height.  none.lib's _screensize wraps this.

screensize:
        ldx     #VID_COLS
        ldy     #VID_ROWS
        rts

; ---------------------------------------------------------------------------
; void __fastcall__ cputcxy (unsigned char x, unsigned char y, char c);

_cputcxy:
        pha                             ; save c
        jsr     gotoxy                  ; pops y and x
        pla
        jmp     _cputc

; ---------------------------------------------------------------------------
; void __fastcall__ cclear   (unsigned char length);
; void __fastcall__ cclearxy (unsigned char x, unsigned char y,
;                             unsigned char length);

_cclearxy:
        pha                             ; save length
        jsr     gotoxy
        pla

_cclear:
        ldy     #' '
        bra     fillrow

; void __fastcall__ chline   (unsigned char length);
; void __fastcall__ chlinexy (unsigned char x, unsigned char y,
;                             unsigned char length);

_chlinexy:
        pha
        jsr     gotoxy
        pla

_chline:
        ldy     #CH_HLINE

; fillrow — A = count, Y = character, written across the current row.

fillrow:
        cmp     #$00
        beq     @done
        sta     tmp1
        sty     tmp2
@loop:  lda     tmp2
        jsr     cputdirect              ; advances the cursor for us
        dec     tmp1
        bne     @loop
@done:  rts

; ---------------------------------------------------------------------------
; void __fastcall__ cvline   (unsigned char length);
; void __fastcall__ cvlinexy (unsigned char x, unsigned char y,
;                             unsigned char length);
;   Down a column instead of across a row, so the character goes down without
;   advancing (VideoPutChar) and the cursor is stepped by hand.  Video only —
;   a column means nothing on a serial console.

_cvlinexy:
        pha
        jsr     gotoxy
        pla

_cvline:
        cmp     #$00
        beq     @done
        sta     tmp1
@loop:  lda     #CH_VLINE
        jsr     VideoPutChar            ; write, leaving the cursor put
        ldx     VID_CURSOR_X
        ldy     VID_CURSOR_Y
        iny                             ; one row down
        jsr     VideoSetCursor
        dec     tmp1
        bne     @loop
@done:  rts

; ---------------------------------------------------------------------------
; unsigned char __fastcall__ revers (unsigned char onoff);
;   Recorded and returned, but the TMS9918 has no reverse attribute in text
;   mode — see the note at the top of this file.

_revers:
        tay                             ; new setting
        ldx     #$00
        lda     reversflag              ; old setting is the result
        sty     reversflag
        rts

; ---------------------------------------------------------------------------
; unsigned char __fastcall__ textcolor   (unsigned char color);
; unsigned char __fastcall__ bgcolor     (unsigned char color);
; unsigned char __fastcall__ bordercolor (unsigned char color);
.ifdef VDP
;   The pen, VID_PEN, is (foreground << 4) | background for text written from
;   now on, and it can be read back.

_textcolor:
        and     #$0F
        asl     a
        asl     a
        asl     a
        asl     a                       ; into the high nibble
        sta     tmp1
        lda     VID_PEN
        pha                             ; keep the old pen
        and     #$0F                    ; hold the background
        ora     tmp1
        jsr     VideoSetColor
        pla
        lsr     a
        lsr     a
        lsr     a
        lsr     a                       ; old foreground
        ldx     #$00
        rts

; The border follows the background, so bordercolor is bgcolor.
_bordercolor:
_bgcolor:
        and     #$0F
        sta     tmp1
        lda     VID_PEN
        pha
        and     #$F0                    ; hold the foreground
        ora     tmp1
        jsr     VideoSetColor
        pla
        and     #$0F                    ; old background
        ldx     #$00
        rts

.else
;   Register 7 holds (foreground << 4) | backdrop for the whole screen and
;   cannot be read back, so colorshadow tracks it.

_textcolor:
        and     #$0F
        asl     a
        asl     a
        asl     a
        asl     a                       ; into the high nibble
        sta     tmp1
        lda     colorshadow
        pha                             ; keep the old pair
        and     #$0F                    ; hold the backdrop
        ora     tmp1
        jsr     setcolor
        pla
        lsr     a
        lsr     a
        lsr     a
        lsr     a                       ; old foreground
        ldx     #$00
        rts

; In text mode the backdrop is also the border, so bordercolor is bgcolor.
_bordercolor:
_bgcolor:
        and     #$0F
        sta     tmp1
        lda     colorshadow
        pha
        and     #$F0                    ; hold the foreground
        ora     tmp1
        jsr     setcolor
        pla
        and     #$0F                    ; old backdrop
        ldx     #$00
        rts

; setcolor — A = the new (fg << 4) | bg pair.
setcolor:
        sta     colorshadow
        jmp     VideoSetColor

.endif

; ---------------------------------------------------------------------------

.ifndef VDP
.data

; Register 7 is write-only, so the shadow has to start at whatever the BIOS
; left there.  InitVideo writes $1F — black text on a white backdrop — and
; nothing in the BIOS changes it afterwards, so that is the boot state.
colorshadow:
        .byte   (TMS_BLACK << 4) | TMS_WHITE
.endif

.bss

reversflag:
        .res    1
