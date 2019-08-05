CCFLAGS := -g -std=c99 -Wall -pedantic

bark: main.c deck.c board.c game.c util.c scoring.c
	gcc $(CCFLAGS) $^ -o bark

one:
	gcc -c $(CCFLAGS) ${FILE} -o bark

debug: CCFLAGS += -D DEBUG
debug: bark
