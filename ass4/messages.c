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
void msg_destroy(Message* message) {
    if (message == NULL) {
        return;
    }
    TRY_FREE(message->data.depotName);

    // free recursive defer message.
    msg_destroy(message->data.deferMessage);
    TRY_FREE(message->data.deferMessage);

    // Material struct is stored wholly inside Message, so no malloc here
    mat_destroy(&message->data.material);

    conn_destroy(message->data.connection);
    TRY_FREE(message->data.connection);
}

/*
// see header
void msgfrom_init(MessageFrom* messageFrom, int port, char* name, 
        Message message) {
    messageFrom->port = port;
    messageFrom->name = strdup(name);
    messageFrom->message = message;
}

// see header
void msgfrom_destroy(MessageFrom* messageFrom) {
    TRY_FREE(messageFrom->name);
    msg_destroy(&messageFrom->message);
}
*/

// see header
char* msg_code(MessageType type) {
    char* msgCodes[NUM_MESSAGE_TYPES_ALL];
    msgCodes[MSG_CONNECT] = "Connect";
    msgCodes[MSG_IM] = "IM";
    msgCodes[MSG_DELIVER] = "Deliver";
    msgCodes[MSG_WITHDRAW] = "Withdraw";
    msgCodes[MSG_TRANSFER] = "Transfer";
    msgCodes[MSG_DEFER] = "Defer";
    msgCodes[MSG_EXECUTE] = "Execute";

    msgCodes[MSG_NULL] = "(null msg type)";
    msgCodes[MSG_META_CONN_NEW] = "(meta conn new)";
    msgCodes[MSG_META_CONN_EOF] = "(meta conn eof)";
    msgCodes[MSG_META_SIGNAL] = "(meta signal)";

    assert(0 <= type && type < NUM_MESSAGE_TYPES_ALL);
    // this is safe because string literals have static lifetime
    return msgCodes[type];
}

// The consume_ family of functions take arguments of a pointer to a pointer
// to the start of the current payload. Each function will modify the value of 
// *start to point _after_ the part it consumes.
// They return a bool of true in success or false on failure.


// consumes a single colon
bool consume_colon(char** start) {
    if (**start != COLON) {
        DEBUG_PRINT("expected colon");
        return false;
    }
    (*start)++; // advance one character
    return true;
}

// consumes a non-negative integer
bool consume_int(char** start, int* outInt) {
    // we can't use parse_int because the number may not span the entire
    // string.
    char* end = NULL;

    // ensures no leading whitespace and number is non-negative
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

// consumes a string until the next colon or end of the string. method is
// sufficiently general to use parameter name outStr.
// MALLOC's the string stored in outStr
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
    // copy exactly len bytes into the new string, and \0.
    strncpy(copy, *start, len + 1);

    *outStr = copy;
    *start = *start + len; // move past this string.
    return true;
}

// consumes a quantity:name pair representing a material.
bool consume_material(char** start, Material* outMaterial) {
    // note we do not use mat_init here because we directly write to 
    // outMaterial.
    return consume_int(start, &outMaterial->quantity) && 
            consume_colon(start) &&
            consume_str(start, &outMaterial->name);
}

// consumes an entire message. useful for recursive messages (Defer).
// MALLOC's a message to store into *outMessage.
bool consume_message(char** start, Message** outMessage) {
    // allocate on stack until we validate the message.
    Message message;
    MessageStatus status = msg_parse(*start, &message);
    if (status != MS_OK) {
        return false;
    }

    // if we get here, message was valid. store new message in heap.
    Message* newMsg = malloc(sizeof(Message));
    *newMsg = message;
    // update the out param with the new message location
    *outMessage = newMsg;
    *start = *start + strlen(*start); // entire string is consumed.
    return true;
}

// consumes the eof, fails if *start is not pointing to \0.
bool consume_eof(char** start) {
    bool valid = **start == '\0';
    if (!valid) {
        DEBUG_PRINT("junk at end of message");
    }
    return valid;
}

// the parse_ family of functions will parse the payload for particular
// message types, storing them into the appropriate fields of the given data
// struct.

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
            consume_material(start, &data->material) &&
            consume_eof(start);
}

/* Parses a Transfer message into the given data struct, returning true on 
 * success.
 */
bool parse_transfer(char* payload, MessageData* data) {
    char** start = &payload;
    return consume_colon(start) && 
            consume_material(start, &data->material) &&
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

// the format_ family of functions all return MALLOC'd strings when given 
// specific data to format. these are (approximately) the inverse of the
// consume_ functions.

// formats an integer and string, colon separated.
char* format_int_str(int number, char* str) {
    return asprintf("%d%c%s", number, COLON, str);
}

// foramts a single integer
char* format_int(int number) {
    return asprintf("%d", number);
}

// formats an int followed by two strings, colon separated
char* format_int_str_str(int number, char* str1, char* str2) {
    return asprintf("%d%c%s%c%s", number, COLON, str1, COLON, str2);
}

// formats a defer payload, taking the key and deferred message
char* format_defer(int key, Message message) {
    char* encoded = msg_encode(message);
    char* ret = asprintf("%d%c%s", key, COLON, encoded);
    free(encoded);
    return ret;
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
    // format_ functions are similar to consume_ in terms of functionality.
    // because we don't really need strict format checks here, we skip the 
    // abstraction of the parse_ methods.
    MessageData data = message.data;
    Material mat = data.material;
    switch (message.type) {
        case MSG_CONNECT:
            return format_int(data.depotPort);
        case MSG_IM:
            return format_int_str(data.depotPort, data.depotName);
        case MSG_DELIVER:
        case MSG_WITHDRAW:
            return format_int_str(mat.quantity, mat.name);
        case MSG_TRANSFER:
            return format_int_str_str(mat.quantity, mat.name, data.depotName);
        case MSG_DEFER:
            // note dereference of deferMessage
            return format_defer(data.deferKey, *data.deferMessage);
        case MSG_EXECUTE:
            return format_int(data.deferKey);
        default:
            assert(0);
    }
}

// see header
MessageStatus msg_parse(char* line, Message* outMessage) {
    Message message = {0};
    message.type = MSG_NULL;
    *outMessage = message; // zero outMessage for safety

    MessageType type = MSG_NULL;
    char* payload = NULL;
    for (int t = 0; t < NUM_MESSAGE_TYPES; t++) {
        char* code = msg_code(t); // get string code expected for this type

        // check prefix of line.
        if (strncmp(line, code, strlen(code)) == 0) {
            type = t;
            // payload is everything after message code.
            payload = line + strlen(code);
            break;
        }
    }
    if (type == MSG_NULL) {
        DEBUG_PRINT("no matched code");
        return MS_INVALID;
    }

    if (!msg_payload_decode(type, payload, &message.data)) {
        DEBUG_PRINT("invalid payload");
        // some incomplete data may be stored, free memory.
        msg_destroy(&message);
        return MS_INVALID;
    }

    message.type = type; // type found
    *outMessage = message; // store parsed message in out param
    return MS_OK;
}

// see header
char* msg_encode(Message message) {
    char* code = msg_code(message.type); // NOT malloc'd
    char* payload = msg_payload_encode(message);

    assert(payload != NULL); // every message has payload now
    char* ret = asprintf("%s%c%s", code, COLON, payload);
    
    free(payload);
    return ret;
}

// see header
MessageStatus msg_receive(FILE* file, Message* outMessage) {
    char* line;
    if (feof(file) || !safe_read_line(file, &line)) {
        DEBUG_PRINT("read pipe is at EOF");
        return MS_EOF;
    }
    DEBUG_PRINTF("received: %s\n", line);

    MessageStatus status = msg_parse(line, outMessage);
    free(line);
    return status;
}

// see header
MessageStatus msg_send(FILE* file, Message message) {
    char* encoded = msg_encode(message);
    DEBUG_PRINTF("sending: %s\n", encoded);

    errno = 0;
    int ret = fprintf(file, "%s\n", encoded);
    fflush(file);

    // DEBUG_PRINTF("errno: %d\n", errno);
    free(encoded);
    // only (reasonable) error is EOF caused by broken pipe
    return (ret >= 0) ? MS_OK : MS_EOF;
}

// see header
void msg_debug(Message* message) {
    MessageData data = message->data;

    DEBUG_PRINTF("msg type: %s\n", msg_code(message->type));
    DEBUG_PRINTF("    data: port=%d, depot=%s, mat={%d, %s}, key=%d, "
            "conn=%p, sig=%d\n", 
            data.depotPort, data.depotName, data.material.quantity, 
            data.material.name, data.deferKey, (void*)data.connection,
            data.signal);
    if (data.deferMessage != NULL) {
        DEBUG_PRINT("deferred message:");
        msg_debug(data.deferMessage);
    }
}

// see header
Message msg_im(int port, char* name) {
    Message msg = {0};
    msg.type = MSG_IM;
    msg.data.depotPort = port;
    msg.data.depotName = strdup(name);
    return msg;
}

// see header
Message msg_deliver(int quantity, char* name) {
    Message msg = {0};
    msg.type = MSG_DELIVER;
    msg.data.material.quantity = quantity;
    msg.data.material.name = strdup(name);
    return msg;
}
