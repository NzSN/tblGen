all:
	mgcc -g -c -o tableGen.o tableGen.c
	mgcc -g -o main main.c tableGen.o
