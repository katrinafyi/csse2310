#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>

#include "messages.h"
#include "util.h"

// tests message related functions
int main(int argc, char** argv) {
    assert(argc >= 2);
    ignore_sigpipe();

    // test msg_receive with first argument.
    int fds[2];
    pipe(fds); // fake a file for msg_receive
    FILE* readFile = fdopen(fds[0], "r");
    FILE* writeFile = fdopen(fds[1], "w");

    fprintf(writeFile, "%s\n", argv[1]);
    fflush(writeFile);

    Message message;
    MessageStatus ret = msg_receive(readFile, &message);
    printf("msg_receive returned: %d\n", ret);
    printf("code: %s\n", msg_code(message.type));

    if (message.type == MSG_HAND) {
        // test encoding hands. LEAKS
        printf("encoded hand: |%s|\n", msg_encode_hand(message.data.hand));
    }

    if (message.type == MSG_PLAYED_CARD) {
        // test encoding tuple
        printf("encoded tuple: |%s|\n", msg_encode_played(
                message.data.playedTuple));
    }

    // test full message sending!
    if (ret == MS_OK) { // but only for valid messages
        printf("send ret: %d\n", msg_send(writeFile, message));
        fflush(writeFile);
        char* line;
        safe_read_line(readFile, &line);
        printf("recv: %s\n", line);
        free(line);
    }
    fclose(readFile); // close and try to write after closed
    fflush(writeFile);
    printf("send after closed: %d\n", msg_send(writeFile, message));
    fclose(writeFile);
    return 0;
}
