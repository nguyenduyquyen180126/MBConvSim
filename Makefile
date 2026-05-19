all: build
build: main.c src/*.c
	gcc main.c src/*.c -fopenmp -o main
clean:
	rm -f main main.exe
