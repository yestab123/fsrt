ALL:
	gcc -c tool.c
	gcc -c send.c
	gcc -o send send.o tool.o -lm -lrt