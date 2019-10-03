#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <errno.h>
#include <ctype.h>

#include "messages.h"
#include "util.h"

// message part separator
#define COLON ':'

// see header
char* msg_code(MessageType type) {
    static char* msgCodes[NUM_MESSAGE_TYPES + 1]; // +1 for NULL code.
    msgCodes[MSG_CONNECT] = "Connect";
    msgCodes[MSG_IM] = "IM";
    msgCodes[MSG_DELIVER] = "Deliver";
    msgCodes[MSG_WITHDRAW] = "Withdraw";
    msgCodes[MSG_TRANSFER] = "Transfer";
    msgCodes[MSG_DEFER] = "Defer";
    msgCodes[MSG_EXECUTE] = "Execute";
    msgCodes[MSG_NULL] = "NULL";

    assert(0 <= type && type < NUM_MESSAGE_TYPES);
    return msgCodes[type];
}

// see header
bool consume_int(char** start, int* outInt) {
    // we can't use parse_int because the number may not span the entire
    // string.
    char* end = NULL;

    if (!isdigit(**start)) {
        DEBUG_PRINT("does not start with digit");
        return false; // string does not start with digit
    }
    errno = 0;
    int val = strtol(*start, &end, 10); // base 10
    if (end == *start || errno != 0) {
        DEBUG_PRINT("strtol error");
        return false; // no digits were parsed or error occured.
    }
    // integer was valid
    *start = end; // move start past the integer parsed.
    *outInt = val;
    return true;
}

// see header
bool consume_str(char** start, char** outStr) {
    int len; // length of the string to consume
    
    // search for colon in the string.
    char* colon = strchr(*start, COLON);
    if (colon == NULL) { // no colon, consume to end of string.
        len = strlen(*start);
    } else { // else, consume up to the colon.
        len = colon - *start;
    }

    if (len == 0) { // empty strings are invalid.
        DEBUG_PRINT("empty string");
        return false;
    }

    // allocate space for a copy of the consumed string.
    char* copy = calloc(len + 1, sizeof(char));
    strncpy(copy, *start, len); // copy exactly len bytes into the new string
    copy[len] = '\0'; // null-terminate string

    *outStr = copy;
    *start = *start + len; // move past this string.
    return true;
}

// see header
bool consume_colon(char** start) {
    if (**start != COLON) {
        DEBUG_PRINT("expected colon");
        return false;
    }
    (*start)++; // advance one character
    return true;
}

// see header
bool consume_message(char** start, Message** outMessagePtr) {
    Message message = {0};
    message.type = MSG_NULL;

    char* line = *start;
    int len = strlen(line);

    MessageType type = MSG_NULL;
    char* payload = NULL;
    for (int t = 0; t < NUM_MESSAGE_TYPES; t++) {
        char* code = msg_code(t); // get string code expected for this type

        // check prefix of line.
        if (strncmp(line, code, strlen(code)) == 0) {
            type = t;
            payload = line + strlen(code);
            break;
        }
    }
    if (type == MSG_NULL) {
        DEBUG_PRINT("no matched code");
        return false;
    }
    message.type = type;

    if (!msg_payload_decode(type, payload, &message.data)) {
        DEBUG_PRINT("invalid payload");
        return false;
    }
    Message* msgPtr = malloc(sizeof(Message));
    *msgPtr = message;
    *outMessagePtr = msgPtr;
    *start = *start + len; // entire string is consumed.
    return true;
}

// see header
bool consume_eof(char** start) {
    bool valid = **start == '\0';
    if (!valid) {
        DEBUG_PRINT("junk at end of message");
    }
    return valid;
}


/* Parses a Connect message into the given data struct, returning true on
 * success.
 */
bool parse_connect(char* payload, MessageData* data) {
    char** start = &payload;
    return consume_colon(start) && consume_int(start, &data->depotPort) &&
        consume_eof(start);
}

/* Parses a IM message into the given data struct, returning true on
 * success.
 */
bool parse_im(char* payload, MessageData* data) {
    char** start = &payload;
    return consume_colon(start) && 
            consume_int(start, &data->depotPort) &&
            consume_colon(start) && 
            consume_str(start, &data->depotName) &&
            consume_eof(start);
}

/* Parses a Deliver/Withdraw message into the given data struct, 
 * returning true on success.
 */
bool parse_deliver_withdraw(char* payload, MessageData* data) {
    char** start = &payload;
    return consume_colon(start) && 
            consume_int(start, &data->material.quantity) &&
            consume_colon(start) &&
            consume_str(start, &data->material.name) && 
            consume_eof(start);
}

/* Parses a Transfer message into the given data struct, returning true on 
 * success.
 */
bool parse_transfer(char* payload, MessageData* data) {
    char** start = &payload;
    return consume_colon(start) && 
            consume_int(start, &data->material.quantity) &&
            consume_colon(start) &&
            consume_str(start, &data->material.name) && 
            consume_colon(start) &&
            consume_str(start, &data->depotName) &&
            consume_eof(start);
}

/* Parses a Defer message into the given data struct, returning true on
 * success.
 */
bool parse_defer(char* payload, MessageData* data) {
    char** start = &payload;
    DEBUG_PRINTF("parsing DEFER payload: %s\n", payload);
    return consume_colon(start) &&
            consume_int(start, &data->deferKey) &&
            consume_colon(start) &&
            consume_message(start, &data->deferMessage) &&
            consume_eof(start);
}

/* Parses an Execute message into the given data struct, returning true on 
 * success.
 */
bool parse_execute(char* payload, MessageData* data) {
    char** start = &payload;
    return consume_colon(start) && consume_int(start, &data->deferKey) &&
            consume_eof(start);
}

// see header
bool msg_payload_decode(MessageType type, char* payload, MessageData* data) {
    bool valid = false;
    switch (type) {
        case MSG_CONNECT:
            valid = parse_connect(payload, data);
            break;
        case MSG_IM:
            valid = parse_im(payload, data);
            break;
        case MSG_DELIVER:
        case MSG_WITHDRAW:
            valid = parse_deliver_withdraw(payload, data);
            break;
        case MSG_TRANSFER:
            valid = parse_transfer(payload, data);
            break;
        case MSG_DEFER:
            valid = parse_defer(payload, data);
            break;
        case MSG_EXECUTE:
            valid = parse_execute(payload, data);
            break;
        default: // shouldn't reach this
            assert(0);
    }
    return valid;
}

// see header
char* msg_payload_encode(Message message) {
    switch (message.type) {
        default:
            assert(0);
    }
}


bool msg_parse(char* line, Message* outMessage) {
    // consume will modify this value. we need to keep the original around
    // to free().
    char** start = &line;
    // consume stores a malloc'd pointer into its argument.
    Message* msgPtr = NULL;
    if (!consume_message(start, &msgPtr)) {
        DEBUG_PRINT("failed to parse");
        return false;
    }
    *outMessage = *msgPtr;
    free(msgPtr);
    return true;
}

// see header
MessageStatus msg_receive(FILE* file, Message* outMessage) {
    *outMessage = (Message) {0}; // zero out messageOut for safety

    char* line;
    if (feof(file) || !safe_read_line(file, &line)) {
        DEBUG_PRINT("read pipe is at EOF");
        return MS_EOF;
    }
    DEBUG_PRINTF("received: %s\n", line);

    if (!msg_parse(line, outMessage)) {
        free(line);
        return MS_INVALID;
    }

    free(line);
    return MS_OK;
}

// see header
MessageStatus msg_send(FILE* file, Message message) {
    char* payload = msg_payload_encode(message);

    errno = 0;
    int ret = fprintf(file, "%s%s\n", msg_code(message.type), payload);
    DEBUG_PRINTF("sending: %s%s\n", msg_code(message.type), payload);
    fflush(file);
    // DEBUG_PRINTF("errno: %d\n", errno);
    free(payload);

    // only (reasonable) error is EOF caused by broken pipe
    return (ret >= 0) ? MS_OK : MS_EOF;
}
