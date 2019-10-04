common_src = util.c array.c messages.c material.c
depot_src = depot.c
all_src = $(common_src) $(depot_src)

common_obj = $(common_src:.c=.o)
depot_obj = $(depot_src:.c=.o)
all_obj = $(all_src:.c=.o)

CCFLAGS := -g -std=gnu99 -Wall -pedantic -pthread
ifeq ($(DEBUG), 1)
    CCFLAGS += -D DEBUG=1
endif

.PHONY: sed_debug sed_noop all debug clean check_debug
release: all

all: 2310depot

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

2310depot: $(common_obj) $(depot_obj)
	gcc $(CCFLAGS) $^ -o $@

clean:
	rm -f *.o 2310depot

sed_debug:
	sed -r -i 's/^(\s+)noop_(PRINTF?)/\1DEBUG_\U\2/gI' $(all_src)

sed_noop:
	sed -r -i 's/^(\s+)DEBUG_(PRINTF?)/\1noop_\L\2/gI' $(all_src)

