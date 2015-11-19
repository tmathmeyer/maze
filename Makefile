all: maze.c
	gcc -Ofast -pedantic-errors maze.c -o maze -lm
	gcc -Ofast -pedantic-errors maze.c -o mazefb -DFRAMEBUFFER -lm

clean:
	rm maze mazefb
