bark: main.c deck.c board.c game.c util.c scoring.c
	gcc $(CCFLAGS) -g -std=c99 -Wall -pedantic $^ -o bark

debug: CCFLAGS += -D DEBUG
debug: bark
