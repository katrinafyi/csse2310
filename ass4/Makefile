common_src = util.c array.c messages.c material.c depotState.c connection.c \
	     exitCodes.c arrayHelpers.c deferGroup.c channel.c network.c
depot_src = main.c
test_src = testUtil.c testMessages.c testArray.c testDepotState.c testDefer.c\
all_src = $(common_src) $(depot_src) $(test_src)

common_h = $(common_src:.c=.h)

common_obj = $(common_src:.c=.o)
depot_obj = $(depot_src:.c=.o)
test_obj = $(test_src:.c=.o)
all_obj = $(all_src:.c=.o)


TERM_GREEN := $(shell tput setaf 2)
TERM_RED := $(shell tput setaf 1)
TERM_RESET := $(shell tput setaf 7)
TERM_REVERSE := "\x1b[7m"

CCFLAGS := -g -std=gnu99 -Wall -pedantic -pthread -Wextra
ifeq ($(DEBUG), 1)
    $(info $(TERM_RED)running in DEBUG mode$(TERM_RESET))
    CCFLAGS += -D DEBUG=1
else
    $(info $(TERM_GREEN)running in RELEASE mode$(TERM_RESET))
endif

.PHONY: sed_debug sed_noop all debug clean check_debug
release: all

all: 2310depot

check_debug:
	./checkDebug.sh $(all_src)

buffer: $(common_obj) buffer.o
	gcc $(CCFLAGS) $^ -o $@

test_util: $(common_obj) testUtil.o
	gcc $(CCFLAGS) $^ -o $@

test_depot: $(common_obj) testDepotState.o
	gcc $(CCFLAGS) $^ -o $@

test_messages: $(common_obj) testMessages.o
	gcc $(CCFLAGS) $^ -o $@

test_array: $(common_obj) testArray.o
	gcc $(CCFLAGS) $^ -o $@

test_defer: $(common_obj) testDefer.o
	gcc $(CCFLAGS) $^ -o $@

%.o: %.c
	gcc $(CCFLAGS) $< -c -o $@

2310depot: $(common_h) $(common_obj) $(depot_obj)
	gcc $(CCFLAGS) $^ -o $@

clean:
	rm -f *.o 2310depot

sed_debug:
	sed -r -i 's/^(\s+)noop_(PRINTF?)/\1DEBUG_\U\2/gI' $(all_src)

sed_noop:
	sed -r -i 's/^(\s+)DEBUG_(PRINTF?)/\1noop_\L\2/gI' $(all_src)

