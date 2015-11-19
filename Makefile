all: maze.c
	gcc -Ofast -pedantic-errors maze.c -o maze

clean:
	rm maze
