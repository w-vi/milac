# MILAC

My first endeavour into to compiler construction. This is a compiler
of a small pascal based school langugae Mila to a slightly modified TINY
machine[^1].

Not really usefull for anything but education, as it is
quite small and employs the basic compiler stuff like, 3 address code,
control flow graph, register alocation (linear scan style) and some
optimizations like common subexpresion elimination, constat folding
and propagation.

See the `test` directory for Mila language reference.

[^1]:
[Louden, K.C. - Compiler Construction, 1997](http://www.cs.sjsu.edu/~louden/cmptext/,
, "Louden, K.C. - Compiler Construction, 1997")

Mila compiler for TINY machine

## Prerequsites

Make
GCC
Flex
Bison


## Build 

`cd src && make && cd ../`
 
For release build with no extra debuging information. Final binary is
copied to root directory.

`cd src && make debug && cd ../`

Debuging release with quite a bit of extra info, cannot be used
without specifing output file as the output is mixed. Final binary is
copied to root directory.


## Help

`mila -h` 
