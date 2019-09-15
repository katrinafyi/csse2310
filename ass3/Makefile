hub_src = hubState.c hub.c
alice_src = playerState.c alice.c
bob_src = playerState.c bob.c
common_src = deck.c util.c exitCodes.c gameState.c messages.c
all_src = $(hub_src) $(alice_src) $(bob_src) $(common_src)

CCFLAGS := -g -std=gnu99 -Wall -pedantic
ifeq ($(DEBUG), 1)
    CCFLAGS += -D DEBUG=1
endif

.PHONY: sed_debug sed_noop all debug 2310hub 2310alice 2310bob
release: sed_noop all

all: 2310hub 2310alice 2310bob

test_compile:
	bash ./testCompile.sh

test_util: $(common_src) testUtil.c
	gcc $(CCFLAGS) $^ -o $@

test_deck: $(common_src) testDeck.c
	gcc $(CCFLAGS) $^ -o $@

test_messages: $(common_src) testMessages.c
	gcc $(CCFLAGS) $^ -o $@

2310hub: $(common_src) $(hub_src)
	gcc $(CCFLAGS) $^ -o $@

2310alice: $(common_src) $(alice_src)
	gcc $(CCFLAGS) $^ -o $@

2310bob: $(common_src) $(bob_src)
	gcc $(CCFLAGS) $^ -o $@

sed_debug:
	sed -r -i 's/^(\s+)noop_(PRINTF?)/\1DEBUG_\U\2/gI' $(all_src)

sed_noop:
	sed -r -i 's/^(\s+)DEBUG_(PRINTF?)/\1noop_\L\2/gI' $(all_src)
