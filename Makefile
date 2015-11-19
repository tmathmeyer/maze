all: maze.c
	gcc -Ofast -pedantic-errors maze.c -o maze
	gcc -Ofast -pedantic-errors maze.c -o mazefb -DFRAMEBUFFER

clean:
	rm maze mazefb
