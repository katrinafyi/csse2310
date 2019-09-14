src = main.c deck.c board.c game.c util.c scoring.c
CCFLAGS := -g -std=c99 -Wall -pedantic
sed_from = DEBUG_
sed_to = noop_\L

release: sed bark

bark:
	gcc $(CCFLAGS) $(src) -o bark

debug: CCFLAGS += -D DEBUG
debug: sed_from = noop_
debug: sed_to = DEBUG_\U
debug: sed bark

sed:
	sed -r -i 's/^(\s+)$(sed_from)(PRINTF?)/\1$(sed_to)\2/gI' $(src)

.PHONY: bark
