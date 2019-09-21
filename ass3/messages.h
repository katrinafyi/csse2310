#ifndef MESSAGES_H
#define MESSAGES_H

#include <stdio.h>
#include <stdbool.h>

#include "deck.h"

// number of valid message types
#define NUM_MESSAGE_TYPES 5

/* Possible message types we can receive, and an extra MSG_NULL to indicate
 * an invalid message type.
 */
typedef enum MessageType {
    MSG_HAND, MSG_NEW_ROUND, MSG_PLAYED_CARD,
            MSG_GAME_OVER, MSG_PLAY_CARD, MSG_NULL
    // style.sh wants weird indentation here
} MessageType;

/* Status which could occur when reading or writing messages.
 */
typedef enum MessageStatus {
    MS_OK, MS_EOF, MS_INVALID
} MessageStatus;

/* Tuple of player and card, used for PLAYED messages. */
typedef struct PlayedTuple {
    int player;
    Card card;
} PlayedTuple;

/* Possible extra data associated with a message, type depends on message type
 */
typedef union MessageData {
    Deck hand; // for HAND
    int leadPlayer; // for NEWROUND
    PlayedTuple playedTuple; // for PLAYED
    Card card; // for PLAY
} MessageData;

/* A message which can be sent or received. Has the given type and data
 * depending on message type.
 */
typedef struct Message {
    MessageType type;
    MessageData data; // data is undefined if type has no extra data!
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
MessageStatus msg_receive(FILE* file, Message* messageOut);

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


/* Decodes a HAND payload message into the given deck,
 * returning true on success, false for format errors.
 */
bool msg_decode_hand(char* payload, Deck* outDeck);

/* Encodes the given deck into a MALLOC'd string.
 */
char* msg_encode_hand(Deck deck);

/* Decodes a single non-negative integer into the given outInt,
 * returning true if successful.
 */
bool msg_decode_int(char* payload, int* outInt);

/* Encodes a single non-negative integer, returning true if successful. */
char* msg_encode_int(int);

/* Decodes a comma-separated pair of player index and card into the given
 * tuple, returning true on success.
 */
bool msg_decode_played(char* payload, PlayedTuple* outTuple);
/* Encodes the given player, card tuple into a string, returning the MALLOC'd
 * string.
 */
char* msg_encode_played(PlayedTuple tuple);

/* Decodes a single card of two characters into the outCard,
 * with rank formatted in lowercase
 * hexadecimal. Returns true on success, false otherwise.
 */
bool msg_decode_card(char* payload, Card* outCard);
/* Encodes the card into a string of suit, then lowercase hex rank.
 * Returns the MALLOC'd string.
 */
char* msg_encode_card(Card card);

/* Returns a HAND message struct with the given hand. */
Message msg_hand(Deck hand);
/* Returns a NEWROUND message with the given lead player. */
Message msg_new_round(int leadPlayer);
/* Returns a PLAYED message with the given player and card. */
Message msg_played_card(int player, Card card);
/* Returns a GAMEOVER message with no data. */
Message msg_game_over(void);
/* Returns a PLAY message with the given card. */
Message msg_play_card(Card card);

#endif
