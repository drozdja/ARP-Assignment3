mkdir -p bin
mkdir -p out
gcc src/master.c -Iinclude -lncurses -lbmp -lrt -lm -lpthread -o bin/master
gcc src/processA.c -Iinclude -lncurses -lbmp -lrt -lpthread -o bin/processA
gcc src/processB.c -Iinclude -lncurses -lbmp -lrt -lpthread -o bin/processB

