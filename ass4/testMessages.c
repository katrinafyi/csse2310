#include <assert.h>
#include "messages.h"
#include "util.h"

int main(int argc, char** argv) {
    assert(argc >= 2);

    Message message = {0};
    if (msg_parse(argv[1], &message) == MS_OK) {
        DEBUG_PRINT("success");
    }
    msg_debug(&message);
    msg_destroy(&message);

    return 0;
}
