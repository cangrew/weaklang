gcc -g LLVMCodegen.c -o codegen  `llvm-config --libs core executionengine interpreter analysis native bitwriter --cflags --ldflags` -lcjson