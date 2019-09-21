hub_src = hubState.c hub.c
alice_src = playerState.c alice.c player.c
bob_src = playerState.c bob.c player.c
common_src = deck.c util.c exitCodes.c gameState.c messages.c
all_src = $(hub_src) $(alice_src) $(bob_src) $(common_src)

hub_obj = $(hub_src:.c=.o)
alice_obj = $(alice_src:.c=.o)
bob_obj = $(bob_src:.c=.o)
common_obj = $(common_src:.c=.o)
all_obj = $(all_src:.c=.o)

CCFLAGS := -g -std=gnu99 -Wall -pedantic
ifeq ($(DEBUG), 1)
    CCFLAGS += -D DEBUG=1
endif

.PHONY: sed_debug sed_noop all debug clean
release: all

all: 2310hub 2310alice 2310bob

check_debug:
	./checkDebug.sh $(all_src)

test_util: $(common_obj) testUtil.o
	gcc $(CCFLAGS) $(common_obj) testUtil.o -o $@

test_deck: $(common_obj) testDeck.o
	gcc $(CCFLAGS) $(common_obj) testDeck.o -o $@

test_messages: $(common_obj) testMessages.o
	gcc $(CCFLAGS) $(common_obj) testMessages.o -o $@

%.o: %.c
	gcc $(CCFLAGS) $< -c -o $@

2310hub: $(common_obj) $(hub_obj) check_debug
	gcc $(CCFLAGS) $(common_obj) $(hub_obj) -o $@

2310alice: $(common_obj) $(alice_obj) check_debug
	gcc $(CCFLAGS) $(common_obj) $(alice_obj) -o $@

2310bob: $(common_obj) $(bob_obj) check_debug
	gcc $(CCFLAGS) $(common_obj) $(bob_obj) -o $@

clean:
	rm -f *.o 2310hub 2310alice 2310bob

sed_debug:
	sed -r -i 's/^(\s+)noop_(PRINTF?)/\1DEBUG_\U\2/gI' $(all_src)

sed_noop:
	sed -r -i 's/^(\s+)DEBUG_(PRINTF?)/\1noop_\L\2/gI' $(all_src)

