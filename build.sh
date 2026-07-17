cd build
rm -rf ./*
cmoc --vectrex --intermediate --verbose ../src/main.c ../src/terminal.c ../src/mul.asm
