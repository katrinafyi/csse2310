#ifndef MESSAGES_H
#define MESSAGES_H

#include <stdio.h>
#include <stdbool.h>

#include "material.h"

// number of valid message types
#define NUM_MESSAGE_TYPES 7

/* Possible message types we can receive, and an extra MSG_NULL to indicate
 * an invalid message type.
 */
typedef enum MessageType {
    MSG_CONNECT, MSG_IM, MSG_DELIVER, MSG_WITHDRAW, MSG_TRANSFER,
    MSG_DEFER, MSG_EXECUTE, MSG_NULL
} MessageType;

/* Status which could occur when reading or writing messages.
 */
typedef enum MessageStatus {
    MS_OK, MS_EOF, MS_INVALID
} MessageStatus;

/* Possible extra data associated with a message, type depends on message type
 * Contains all fields always, but only the appropriate fields will be set
 * depending on message type.
 */
typedef struct MessageData {
    int depotPort; // depot port from IM
    char* depotName; // depot name from IM

    Material material; // material and quantity
    char* destName; // transfer destination

    int deferKey; // key for defer/execute
    struct Message* deferMessage; // submessage for deferred messages
} MessageData;

/* A message which can be sent or received. Has the given type and data
 * depending on message type.
 */
typedef struct Message {
    MessageType type;
    struct MessageData data; // data is undefined if type has no extra data!
} Message;



/* Returns the string message code associated with the given message type.
 *
 * Returned string is static and MUST NOT be free'd.
 */
char* msg_code(MessageType type);

/* Receives a message from the given file. Message should be terminated with
 * a newline or EOF. If EOF is received immediately, MS_EOF is returned.
 * MS_INVALID is returned if the message is incorrectly formatted. Otherwise,
 * the parsed message struct is stored into messageOut and MS_OK is returned.
 *
 * Warning: this can allocate memory if the message data type requires it,
 * e.g. HAND. This will leak if the caller is not equipped to handler that
 * message type.
 */
MessageStatus msg_receive(FILE* file, Message* outMessage);

/* Parses the given message into outMessage. As above but input is taken from
 * given string instead of the file. Returns true on success, false otherwise.
 */
bool msg_parse(char* line, Message* outMessage);

/* Sends a message to the given file. message is assumed to be correct and will
 * be sent (MS_INVALID is never returned).
 *
 * Returns MS_OK on success, MS_EOF on failure.
 * Note that this does not consistently detect a broken pipe!
 */
MessageStatus msg_send(FILE* file, Message message);

/* Decodes the given payload string into the given data struct, assuming
 * the message is of the given type.
 *
 * Returns true on success, false for any formatting errors.
 */
bool msg_payload_decode(MessageType type, char* payload, MessageData* data);

/* Encodes the data of the given message into a payload string, returning
 * the payload string. Message type and data are contained in the message
 * struct.
 *
 * Returns a MALLOC'd string.
 */
char* msg_payload_encode(Message message);

// The consume_ family of functions take arguments of the current payload
// being parsed. Each function will modify the value of *start to point to
// the part _after_ the parsed content. 

// They return a bool of true in success or false on failure.

// consumes a non-negative integer
bool consume_int(char** start, int* outInt);
// consumes a string until the next colon. stores a MALLOC'd string into
// *outStr.
bool consume_str(char** start, char** outStr);
// consumes a single colon
bool consume_colon(char** start);
// consumes an entire message
bool consume_message(char** start, Message** outMessage);
// consumes trailing \0. that is, ensures message is fully parsed.
bool consume_eof(char** start);

#endif
