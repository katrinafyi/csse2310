#include <assert.h>
#include "messages.h"

int main(int argc, char** argv) {
    assert(argc >= 2);

    Message message;
    msg_parse(argv[1], &message);
    return 0;
}
