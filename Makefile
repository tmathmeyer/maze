all: maze.c
	gcc -Ofast -std=c11 -pedantic-errors maze.c -o maze

clean:
	rm maze
