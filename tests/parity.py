#!/usr/bin/env python3
# =============================================================================
#   parity.py — 6502.h and 6502.inc name the same addresses
# =============================================================================
#
#   The two headers are meant to be the same document in two languages, and
#   the addresses are the part that silently rots: change an equate in one and
#   nothing complains until a program writes to the wrong register.
#
#   This pulls every address symbol out of both files and compares the ones
#   that appear in each.  A symbol in only one file is not an error — the
#   Kernal jump table is prototypes in 6502.h and equates in 6502.inc, and
#   that is deliberate — so only genuine disagreements fail the run.
#
#   Usage:  parity.py 6502.inc 6502.h [-v]
#
# =============================================================================

import re
import sys

# ca65:  NAME := $ADDR
INC_RE = re.compile(r'^([A-Za-z_]\w*)\s*:=\s*\$([0-9A-Fa-f]+)')

# C:     #define NAME (*(volatile type *)0xADDR)   /  ((type *)0xADDR)
HDR_RE = re.compile(r'^#define\s+([A-Za-z_]\w*)\s+\(+\*?\(.*?\)\s*0x([0-9A-Fa-f]+)\)')


def scan(path, pattern):
    out = {}
    with open(path, encoding='utf-8') as f:
        for line in f:
            m = pattern.match(line)
            if m:
                out[m.group(1)] = int(m.group(2), 16)
    return out


def main():
    args = [a for a in sys.argv[1:] if a != '-v']
    verbose = '-v' in sys.argv
    if len(args) != 2:
        print(__doc__ or 'usage: parity.py 6502.inc 6502.h [-v]')
        return 2

    inc = scan(args[0], INC_RE)
    hdr = scan(args[1], HDR_RE)

    shared = sorted(set(inc) & set(hdr))
    bad = [n for n in shared if inc[n] != hdr[n]]

    for n in bad:
        print(f'MISMATCH {n}: {args[0]} ${inc[n]:04X} vs {args[1]} 0x{hdr[n]:04X}')

    print(f'{len(shared)} address symbols compared, {len(bad)} mismatched')

    if verbose:
        only_inc = sorted(set(inc) - set(hdr))
        only_hdr = sorted(set(hdr) - set(inc))
        if only_inc:
            print(f'\nonly in {args[0]} ({len(only_inc)}):\n  ' + '\n  '.join(only_inc))
        if only_hdr:
            print(f'\nonly in {args[1]} ({len(only_hdr)}):\n  ' + '\n  '.join(only_hdr))

    return 1 if bad else 0


if __name__ == '__main__':
    sys.exit(main())
