#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <unistd.h>

#include "messages.h"

// tests message related functions
int main(int argc, char** argv) {
    assert(argc >= 2);

    // test msg_receive with first argument.
    int fds[2];
    pipe(fds); // fake a file for msg_receive
    FILE* readFile = fdopen(fds[0], "r");
    FILE* writeFile = fdopen(fds[1], "w");

    fprintf(writeFile, "%s\n", argv[1]);
    fflush(writeFile);
    fclose(writeFile);

    Message message;
    MessageStatus ret = msg_receive(readFile, &message);
    assert(fgetc(readFile) == EOF);
    printf("msg_receive returned: %d\n", ret);
    printf("code: %s\n", msg_code(message.type));

    if (message.type == MSG_HAND) {
        // test encoding hands
        printf("encoded hand: |%s|\n", msg_encode_hand(message.data.hand));
    }

    if (message.type == MSG_PLAYED_CARD) {
        // test encoding tuple
        printf("encoded tuple: |%s|\n", msg_encode_played(
                message.data.playedTuple));
    }

    return 0;
}
