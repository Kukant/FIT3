

all: tmal.c test_tmal.c
		gcc -std=c99 -g -Wall -Wextra tmal.c -c
		gcc -std=c99 -g  -Wall -Wextra test_tmal.c -c
		gcc -o test_tmal test_tmal.o tmal.o
		rm *.o
