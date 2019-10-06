#include <assert.h>
#include "messages.h"
#include "util.h"

int main(int argc, char** argv) {
    assert(argc >= 2);

    Message message = {0};
    if (msg_parse(argv[1], &message) == MS_OK) {
        printf("success, trying to re-encode\n");
        char* encoded = msg_encode(message);
        printf("encoded: %s\n", encoded);
        free(encoded);
    }


    msg_debug(&message);
    msg_destroy(&message);

    return 0;
}
