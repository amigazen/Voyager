# libmath64

A SAS/C math linklib with 64-bit support

© 2001-2024 by David Gerber
All Rights Reserved

This is a simple math library with 64-bit support for SAS/C.
It supports 68020+ processors. Support for 68000 is possible
but not planned. 68020-040 instructions are emulated by
68060.library (slow) or patched by something like
CyberPatcher (fast).

## Currently suported

68020-060:
- qtod(): QWORD to double conversion
- uqtoud(): UQWORD to double conversion
- q_add(): QWORD addition
- q_sub(): QWORD substraction
- q_div(): QWORD division
- q_divu(): UQWORD division
- q_div64(): QWORD division with 64-bit result
- q_divu64(): UQWORD division with 64-bit result
- q_mod(): QWORD modulo
- q_modu(): UQWORD modulo
- q_mulu(): ULONG multiplication with 64-bit result

68020-040:
- q_div020(): hardware QWORD division
- q_divu020(): hardware UQWORD division
- q_mod020(): hardware QWORD modulo
- q_mul020(): hardware UQWORD modulo

## Todo

- rewrite the q_div() functions in assembler
