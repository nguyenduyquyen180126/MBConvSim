all: build
build: main.c
	gcc main.c -fopenmp
clean:
	rm -r *.exe
