# weaklang
Compiler for weaklang.  A language inspired by PL/0 and used for compiler development practice.

## Structure
Flex is used as Lexer.
Bison is used for Parsing.
LLVM is used for Object Code Generation.

## Example Program
```
const a = 1, b = 1;
var x,y,z;
def main {
    y = 1;
    if y == 1 then
        print(y);
}
```

## Features
Currently only implements variable assignment and arithmetic, if blocks and I/O Output.
