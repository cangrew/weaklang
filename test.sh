bison -d weaklang.y
flex weaklang.l
gcc -g driver.c lex.yy.c weaklang.tab.c LLVMCodegen.c -o simplelang -lfl -lcjson `llvm-config --libs core executionengine interpreter analysis native bitwriter --cflags --ldflags`
./simplelang in.wk
gcc output.o -o test