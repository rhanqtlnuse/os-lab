main: main.o my_print.o
	gcc -static -fPIC -o main main.o my_print.o
main.o: main.c
	gcc -c main.c
clean:
	rm main.o
