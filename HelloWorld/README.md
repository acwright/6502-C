Hello World
===========

Prints "Hello, World!" to the output device, in C.

The assembly version of this program lives in
[6502-ASM](https://github.com/acwright/6502-ASM); this is the same program
written for cc65, and the same `.prg` loaded at `$0800` comes out the other
end. It prints once through the Kernal (`PrintStr`) and once through the C
library (`printf`) so you can see both paths, and what each one costs.

Install:

    brew install cc65

Build:

    make

View:

    make view

Size:

    make size

Run:

    make run
