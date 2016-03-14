all: maze.c
	gcc -Ofast -pedantic-errors maze.c gradient.c -o maze
	gcc -Ofast -pedantic-errors maze.c gradient.c -o mazefb -DFRAMEBUFFER

grad:
	gcc -g -o gradient gradient.c

clean:
	rm maze mazefb
