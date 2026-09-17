#!/usr/bin/env python3
# =============================================================================
#   parity.py — a C header and its ca65 include name the same things
# =============================================================================
#
#   The two headers are meant to be the same document in two languages, and
#   the values are the part that silently rots: change an equate in one and
#   nothing complains until a program writes to the wrong register.
#
#   Two comparisons, over every name that appears in both files:
#
#     addresses   NAME := $ADDR       against  #define NAME (*(type *)0xADDR)
#     constants   NAME = value        against  #define NAME value
#                 (also NAME := value where the header's is a plain number)
#
#   A symbol in only one file is not an error — the Kernal jump table is
#   prototypes in the header and equates in the include, and that is
#   deliberate — so only genuine disagreements fail the run.  The constants
#   matter most for 6502-VDP.inc / 6502-VDP.h, where register numbers and bit
#   fields are constants.
#
#   Usage:  parity.py 6502.inc 6502.h [-v]
#           parity.py 6502-VDP.inc 6502-VDP.h [-v]
#
# =============================================================================

import re
import sys

NUMBER = r'(\$[0-9A-Fa-f]+|%[01]+|[0-9]+)'

# ca65:  NAME := $ADDR
INC_ADDR_RE = re.compile(r'^([A-Za-z_]\w*)\s*:=\s*\$([0-9A-Fa-f]+)')

# ca65:  NAME = value   /   NAME := value   (a literal number)
INC_CONST_RE = re.compile(r'^([A-Za-z_]\w*)\s*:?=\s*' + NUMBER + r'\s*(?:;.*)?$')

# C:     #define NAME (*(volatile type *)0xADDR)   /  ((type *)0xADDR)
HDR_ADDR_RE = re.compile(r'^#define\s+([A-Za-z_]\w*)\s+\(+\*?\(.*?\)\s*0x([0-9A-Fa-f]+)\)')

# C:     #define NAME 0x1F   /   #define NAME 40
HDR_CONST_RE = re.compile(r'^#define\s+([A-Za-z_]\w*)\s+(0x[0-9A-Fa-f]+|[0-9]+)\s*(?:/\*.*)?$')


def number(text):
    if text.startswith('$'):
        return int(text[1:], 16)
    if text.startswith('%'):
        return int(text[1:], 2)
    return int(text, 0) if text.lower().startswith('0x') else int(text)


def scan(path, pattern, convert):
    out = {}
    with open(path, encoding='utf-8') as f:
        for line in f:
            m = pattern.match(line)
            if m:
                out[m.group(1)] = convert(m.group(2))
    return out


def compare(label, inc, hdr, paths, verbose):
    shared = sorted(set(inc) & set(hdr))
    bad = [n for n in shared if inc[n] != hdr[n]]
    for n in bad:
        print(f'MISMATCH {n}: {paths[0]} ${inc[n]:04X} vs {paths[1]} 0x{hdr[n]:04X}')
    print(f'{len(shared)} {label} compared, {len(bad)} mismatched')
    if verbose:
        only_inc = sorted(set(inc) - set(hdr))
        only_hdr = sorted(set(hdr) - set(inc))
        if only_inc:
            print(f'\n{label} only in {paths[0]} ({len(only_inc)}):\n  ' + '\n  '.join(only_inc))
        if only_hdr:
            print(f'\n{label} only in {paths[1]} ({len(only_hdr)}):\n  ' + '\n  '.join(only_hdr))
    return len(bad)


def main():
    args = [a for a in sys.argv[1:] if a != '-v']
    verbose = '-v' in sys.argv
    if len(args) != 2:
        print('usage: parity.py INCLUDE HEADER [-v]')
        return 2

    inc_addr = scan(args[0], INC_ADDR_RE, lambda h: int(h, 16))
    hdr_addr = scan(args[1], HDR_ADDR_RE, lambda h: int(h, 16))
    hdr_const = scan(args[1], HDR_CONST_RE, number)
    # An include equate the header spells as a pointer was an address above.
    inc_const = {n: v for n, v in scan(args[0], INC_CONST_RE, number).items()
                 if n not in hdr_addr}

    bad = compare('address symbols', inc_addr, hdr_addr, args, verbose)
    bad += compare('constants', inc_const, hdr_const, args, verbose)
    return 1 if bad else 0


if __name__ == '__main__':
    sys.exit(main())
