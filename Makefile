all: build
build: main.c
	gcc -I./include src/*.c main.c -fopenmp
clean:
	rm -r *.exe
