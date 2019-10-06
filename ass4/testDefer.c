#include <stdlib.h>
#include <stdio.h>

#include "deferGroup.h"
#include "messages.h"

// prints a single Message*.
void print_msg(void* msg) {
    msg_debug(msg);
    fprintf(stderr, "-----\n");
}

// test defer group
int main(int argc, char** argv) {
    DeferGroup dgData = {0};
    DeferGroup* dg = &dgData;
    dg_destroy(dg); // test destroy on null struct
    dg_init(dg, 10);

    Message msg;
    msg_parse("Deliver:1:wood", &msg);
    dg_add_message(dg, msg);
    msg_parse("Transfer:20:paper:home", &msg);
    dg_add_message(dg, msg);

    array_foreach(dg->messages, print_msg);

    dg_destroy(dg);
    return 0;
}
