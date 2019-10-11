#ifndef MESSAGES_H
#define MESSAGES_H

#include <stdio.h>
#include <stdbool.h>

#include "channel.h"
#include "material.h"

// number of valid message types
#define NUM_MESSAGE_TYPES 7
// number of all message types
#define NUM_MESSAGE_TYPES_ALL 11

/* Possible message types we can receive and other special flags for
 * indicating specific state transitions
 */
typedef enum MessageType {
    // messages defined in spec
    MSG_CONNECT, 
    MSG_IM, 
    MSG_DELIVER, 
    MSG_WITHDRAW, 
    MSG_TRANSFER, 
    MSG_DEFER, 
    MSG_EXECUTE, 
    
    // past this are various meta messages
    
    MSG_NULL, 
    // new connection established and verified. data contains a channel to 
    // write _to_ this connection.
    MSG_META_CONN_NEW, 
    MSG_META_CONN_EOF, // connection terminated
    MSG_META_SIGNAL // signal received. data contains signal number
} MessageType;

/* Status which could occur when reading or writing messages.
 */
typedef enum MessageStatus {
    MS_OK, // message received and parsed successfully
    MS_EOF, // eof was encountered before any bytes were received
    MS_INVALID // message received was of an invalid format
} MessageStatus;

/* Possible extra data associated with a message, type depends on message type
 * Contains all fields always, but only the appropriate fields will be set
 * depending on message type.
 */
typedef struct MessageData {
    int depotPort; // depot port from IM
    char* depotName; // MALLOC! depot name from IM or destination

    Material material; // mat_destroy! material and quantity

    int deferKey; // key for defer/execute
    struct Message* deferMessage; // MALLOC! submessage for deferred messages
    
    // meta message things

    // MALLOC! channel for sending messages to this connection.
    Channel* channel; 
    int signal; // signal received
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

/* Sends a message to the given file. message is assumed to be correct and will
 * be sent (MS_INVALID is never returned).
 *
 * Returns MS_OK on success, MS_EOF on failure.
 * Note that this does not consistently detect a broken pipe!
 */
MessageStatus msg_send(FILE* file, Message message);

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
 * given string instead of the file. Returns status of message parsing, MS_EOF 
 * will never be returned.
 */
MessageStatus msg_parse(char* line, Message* outMessage);

/* Encodes the given message into the string representing that message.
 * Assumes the message is valid; a MALLOC'd string will always be returned.
 */
char* msg_encode(Message message);

/* Destroys the given message, freeing its contained structures and setting
 * pointers to NULL.
 */
void msg_destroy(Message* message);


///* Initialises a new MessageFrom with the given port, name and message.
// * Copies name into a new MALLOC'd string and takes ownership of the given
// * message.
// */
//void msgfrom_init(MessageFrom* messageFrom, int port, char* name, 
//        Message message);
//
///* Destroys the given MessageFrom, freeing its memory and message.
// */
//void msgfrom_destroy(MessageFrom* messageFrom);

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

/* Prints the message in an (arbitrary) format for debugging,
 * if debug is enabled.
 */
void msg_debug(Message* message);

/* Creates a new IM message with the given port and depot name, returning the
 * message.
 */
Message msg_im(int port, char* name);

/* Creates a new Deliver message for the given material, returning the message
 */
Message msg_deliver(int quantity, char* name);

#endif
