#ifndef MESSAGES_H
#define MESSAGES_H

#include "deck.h"

#define NUM_MESSAGE_TYPES 5

typedef enum {
    MSG_HAND, MSG_NEW_ROUND, MSG_PLAYED_CARD,
    MSG_GAME_OVER, MSG_PLAY_CARD, MSG_NULL
} MessageType;

typedef enum {
    MS_OK, MS_EOF, MS_INVALID
} MessageStatus;

typedef struct {
    int player;
    Card card;
} PlayedTuple;

typedef union {
    Deck hand;
    int leadPlayer;
    PlayedTuple playedTuple;
    Card card;
} MessageData;

typedef struct {
    MessageType type;
    MessageData data;
} Message;

char* msg_code(MessageType type);

MessageStatus msg_receive(FILE* file, Message* messageOut);

MessageStatus msg_send(FILE* file, Message message);

bool msg_decode_hand(char* payload, Deck* outDeck);
char* msg_encode_hand(Deck deck);

bool msg_decode_int(char* payload, int* outInt);
char* msg_encode_int(int);

bool msg_decode_played(char* payload, PlayedTuple* outTuple);
char* msg_encode_played(PlayedTuple tuple);

bool msg_decode_card(char* payload, Card* outCard);
char* msg_encode_card(Card card);


#endif
