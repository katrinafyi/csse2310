bark: main.c deck.c board.c game.c util.c
	gcc -g -D DEBUG -std=c99 -Wall -pedantic $^ -o bark

build:
	$(MAKE) bark

run:
	$(MAKE) build
	./bark
