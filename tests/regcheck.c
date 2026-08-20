/* =============================================================================
 *   regcheck.c — the IO register blocks agree with the Section 5 macros
 * =============================================================================
 *
 *   Section 5b of 6502.h describes each IO card twice: once as a flat macro
 *   per address, and once as a struct overlaid on the card's base address.
 *   Nothing in the language ties the two together, so a field inserted in the
 *   wrong place, or a padding byte forgotten, would silently move every field
 *   below it onto the wrong register — and the program would still compile.
 *
 *   This file makes that a build error instead.  SAME() takes the address of
 *   a struct field and of the macro for the same register and sizes an array
 *   by whether they match, so a wrong offset gives:
 *
 *       Error: Size of array 'chk' is invalid
 *
 *   Nothing here is linked or run.  `make check` compiles it and throws the
 *   object away; the cost is one cc65 invocation.
 *
 * =============================================================================
 */

#include "6502.h"

#define SAME(a, b)  extern char chk[(&(a) == &(b)) ? 1 : -1]

/* --- Banked RAM Low / High (IO 1, IO 2) --- */
SAME(RAM_L.data[0],  RAM_DATA_L[0]);
SAME(RAM_L.data[1022], RAM_DATA_L[1022]);
SAME(RAM_L.bank,     RAM_BANK_L);
SAME(RAM_H.data[0],  RAM_DATA_H[0]);
SAME(RAM_H.bank,     RAM_BANK_H);

/* --- RTC (IO 3) --- */
SAME(RTC.sec,        RTC_SEC);
SAME(RTC.min,        RTC_MIN);
SAME(RTC.hr,         RTC_HR);
SAME(RTC.day,        RTC_DAY);
SAME(RTC.date,       RTC_DATE);
SAME(RTC.mon,        RTC_MON);
SAME(RTC.yr,         RTC_YR);
SAME(RTC.cent,       RTC_CENT);
SAME(RTC.alarm_sec,  RTC_ALARM_SEC);
SAME(RTC.alarm_min,  RTC_ALARM_MIN);
SAME(RTC.alarm_hr,   RTC_ALARM_HR);
SAME(RTC.alarm_date, RTC_ALARM_DATE);
SAME(RTC.wd_l,       RTC_WD_L);
SAME(RTC.wd_h,       RTC_WD_H);
SAME(RTC.ctrl_a,     RTC_CTRL_A);
SAME(RTC.ctrl_b,     RTC_CTRL_B);
SAME(RTC.ram_addr,   RTC_RAM_ADDR);
SAME(RTC.ram_data,   RTC_RAM_DATA);   /* the $8811-$8812 gap is padded */

/* --- CompactFlash / Storage (IO 4) --- */
SAME(ST.data,        ST_DATA);
SAME(ST.error,       ST_ERROR);
SAME(ST.feature,     ST_FEATURE);     /* same port as .error */
SAME(ST.sect_cnt,    ST_SECT_CNT);
SAME(ST.lba_0,       ST_LBA_0);
SAME(ST.lba_1,       ST_LBA_1);
SAME(ST.lba_2,       ST_LBA_2);
SAME(ST.lba_3,       ST_LBA_3);
SAME(ST.lba[0],      ST_LBA_0);       /* array view of the same four */
SAME(ST.lba[3],      ST_LBA_3);
SAME(ST.status,      ST_STATUS);
SAME(ST.cmd,         ST_CMD);         /* same port as .status */

/* --- Serial (IO 5) --- */
SAME(SC.data,        SC_DATA);
SAME(SC.status,      SC_STATUS);
SAME(SC.reset,       SC_RESET);       /* same port as .status */
SAME(SC.cmd,         SC_CMD);
SAME(SC.ctrl,        SC_CTRL);

/* --- GPIO / VIA (IO 6) --- */
SAME(GPIO.portb,     GPIO_PORTB);
SAME(GPIO.porta,     GPIO_PORTA);
SAME(GPIO.ddrb,      GPIO_DDRB);
SAME(GPIO.ddra,      GPIO_DDRA);
SAME(GPIO.t1cl,      GPIO_T1CL);
SAME(GPIO.t1ch,      GPIO_T1CH);
SAME(GPIO.t1ll,      GPIO_T1LL);
SAME(GPIO.t1lh,      GPIO_T1LH);
SAME(GPIO.t2cl,      GPIO_T2CL);
SAME(GPIO.t2ch,      GPIO_T2CH);
SAME(GPIO.sr,        GPIO_SR);
SAME(GPIO.acr,       GPIO_ACR);
SAME(GPIO.pcr,       GPIO_PCR);
SAME(GPIO.ifr,       GPIO_IFR);
SAME(GPIO.ier,       GPIO_IER);
SAME(GPIO.ora,       GPIO_ORA);

/* The 16-bit timer names must start on the low byte of their pair. */
SAME(*(volatile unsigned char *)&GPIO.t1c, GPIO_T1CL);
SAME(*(volatile unsigned char *)&GPIO.t1l, GPIO_T1LL);
SAME(*(volatile unsigned char *)&GPIO.t2c, GPIO_T2CL);

/* --- SID (IO 7) --- */
SAME(SID.v1.freq_lo, SID_V1_FREQ_LO);
SAME(SID.v1.freq_hi, SID_V1_FREQ_HI);
SAME(SID.v1.pw_lo,   SID_V1_PW_LO);
SAME(SID.v1.pw_hi,   SID_V1_PW_HI);
SAME(SID.v1.ctrl,    SID_V1_CTRL);
SAME(SID.v1.ad,      SID_V1_AD);
SAME(SID.v1.sr,      SID_V1_SR);
SAME(SID.v2.freq_lo, SID_V2_FREQ_LO);
SAME(SID.v2.pw_hi,   SID_V2_PW_HI);
SAME(SID.v2.ctrl,    SID_V2_CTRL);
SAME(SID.v2.sr,      SID_V2_SR);
SAME(SID.v3.freq_lo, SID_V3_FREQ_LO);
SAME(SID.v3.pw_lo,   SID_V3_PW_LO);
SAME(SID.v3.ctrl,    SID_V3_CTRL);
SAME(SID.v3.sr,      SID_V3_SR);

/* The array view and the named view must be the same three voices. */
SAME(SID.voice[0].freq_lo, SID_V1_FREQ_LO);
SAME(SID.voice[1].freq_lo, SID_V2_FREQ_LO);
SAME(SID.voice[2].freq_lo, SID_V3_FREQ_LO);
SAME(SID.voice[2].sr,      SID_V3_SR);

/* 16-bit names start on the low byte of their pair. */
SAME(*(volatile unsigned char *)&SID.v1.freq, SID_V1_FREQ_LO);
SAME(*(volatile unsigned char *)&SID.v1.pw,   SID_V1_PW_LO);
SAME(*(volatile unsigned char *)&SID.fc,      SID_FC_LO);

SAME(SID.fc_lo,      SID_FC_LO);
SAME(SID.fc_hi,      SID_FC_HI);
SAME(SID.res_filt,   SID_RES_FILT);
SAME(SID.mode_vol,   SID_MODE_VOL);
SAME(SID.paddle_x,   SID_PADDLE_X);
SAME(SID.paddle_y,   SID_PADDLE_Y);
SAME(SID.osc3,       SID_OSC3);
SAME(SID.env3,       SID_ENV3);

/* --- Video (IO 8) --- */
SAME(VC.data,        VC_DATA);
SAME(VC.reg,         VC_REG);
SAME(VC.status,      VC_STATUS);      /* same port as .reg */

/* --- Each block must be exactly as wide as the card's decoded window. --- */
extern char sz_ram [sizeof(struct __ac_ram)       == 1024 ? 1 : -1];
extern char sz_rtc [sizeof(struct __ac_rtc)       == 0x14 ? 1 : -1];
extern char sz_st  [sizeof(struct __ac_st)        == 8    ? 1 : -1];
extern char sz_sc  [sizeof(struct __ac_sc)        == 4    ? 1 : -1];
extern char sz_gpio[sizeof(struct __ac_gpio)      == 16   ? 1 : -1];
extern char sz_voic[sizeof(struct __ac_sid_voice) == 7    ? 1 : -1];
extern char sz_sid [sizeof(struct __ac_sid)       == 0x1D ? 1 : -1];
extern char sz_vc  [sizeof(struct __ac_vc)        == 2    ? 1 : -1];
