#ifndef MESSAGES_H
#define MESSAGES_H

#define NUM_MESSAGE_TYPES 5

typedef enum {
    HAND, NEW_ROUND, PLAYED_CARD,
    GAME_OVER, PLAY_CARD
} MessageType;

typedef enum {
    OK, EOF, INVALID
} MessageStatus;

typedef struct {
    int player;
    Card card;
} PlayedTuple;

typedef union {
    Deck deck;
    int leadPlayer;
    PlayedTuple playedTuple;
} MessageData;

typedef struct {
    MessageType type;
    MessageData data;
} Message;

char* get_message_code(MessageType type);

MessageStatus msg_receive(FILE* file, Message* messageOut);

// payload = encoded data
Deck msg_decode_hand(char* payload);
char* msg_encode_hand(Deck deck);

int msg_decode_int(char* payload);
char* msg_encode_int(int);

PlayedTuple msg_decode_played(char* payload);
char* msg_encode_played(PlayedTuple tuple);

bool msg_send(FILE* file, Message message);

#endif
