#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <errno.h>

#include "messages.h"
#include "util.h"
#include "deck.h"



// see header
char* msg_code(MessageType type) {
    static char* typeStrings[6] = {
        "HAND",
        "NEWROUND",
        "PLAYED",
        "GAMEOVER",
        "PLAY",
        "NULL" // internal use only
    };
    assert(0 <= type && type < 6);
    return typeStrings[type];
}


bool msg_payload_decode(MessageType type, char* payload, MessageData* data) {
    bool valid = false;
    switch (type) {
        case MSG_HAND:
            valid = msg_decode_hand(payload, &data->hand);
            break;
        case MSG_NEW_ROUND:
            valid = msg_decode_int(payload, &data->leadPlayer);
            break;
        case MSG_PLAYED_CARD:
            valid = msg_decode_played(payload, &data->playedTuple);
            break;
        case MSG_PLAY_CARD:
            valid = msg_decode_card(payload, &data->card);
            break;
        default: // other messages have no body
            valid = payload[0] == '\0';
            break;
    }
    return valid;
}

char* msg_payload_encode(Message message) {
    switch (message.type) {
        case MSG_HAND:
            return msg_encode_hand(message.data.hand);
        case MSG_NEW_ROUND:
            return msg_encode_int(message.data.leadPlayer);
        case MSG_PLAYED_CARD:
            return msg_encode_played(message.data.playedTuple);
        case MSG_PLAY_CARD:
            return msg_encode_card(message.data.card);
        default:
            return strdup(""); // mallocs an empty string
    }
}


// see header
MessageStatus msg_receive(FILE* file, Message* outMessage) {
    Message message = {0};
    message.type = MSG_NULL;
    *outMessage = message; // zero out messageOut for safety

    char* line;
    if (feof(file) || !safe_read_line(file, &line)) {
        return MS_EOF;
    }

    MessageType type = MSG_NULL;
    char* payload = NULL;
    for (int t = 0; t < NUM_MESSAGE_TYPES; t++) {
        char* code = msg_code(t);
        // warning: PLAY and PLAYED have a common prefix
        // so played must be checked first and we must break on match.
        if (strncmp(line, code, strlen(code)) == 0) {
            type = t;
            payload = line + strlen(code);
            break;
        }
    }
    if (type == MSG_NULL) {
        DEBUG_PRINT("no matched code");
        free(line);
        return MS_INVALID;
    }
    if (!msg_payload_decode(type, payload, &message.data)) {
        DEBUG_PRINT("invalid payload");
        free(line);
        return MS_INVALID;
    }
    free(line);
    message.type = type;
    *outMessage = message;
    return MS_OK;
}

// see header
MessageStatus msg_send(FILE* file, Message message) {
    // only (reasonable) error is EOF caused by broken pipe
    char* payload = msg_payload_encode(message);
    // note newline at end
    errno = 0;
    int ret = fprintf(file, "%s%s\n", msg_code(message.type), payload);
    // DEBUG_PRINTF("errno: %d\n", errno);
    free(payload);
    return (ret >= 0) ? MS_OK : MS_EOF;
}

// see header
bool msg_decode_hand(char* payload, Deck* outDeck) {
    char* firstSplit[2];
    // first, split into number of cards and the rest.
    if (tokenise(payload, ',', firstSplit, 2) != 2) {
        DEBUG_PRINT("couldn't find header");
        return false;
    }
    int numCards = parse_int(firstSplit[0]);
    if (numCards <= 0) {
        DEBUG_PRINT("invalid num cards value");
        return false;
    }
    deck_init_empty(outDeck, numCards);

    // split on each comma and parse cards
    // one more allocated to make sure no junk is at end of string
    char** cardsSplit = calloc(numCards, sizeof(char*));
    if (tokenise(firstSplit[1], ',', cardsSplit, numCards) != numCards) {
        DEBUG_PRINT("wrong number of cards");
        free(cardsSplit);
        deck_destroy(outDeck);
        return false;
    }

    for (int i = 0; i < numCards; i++) {
        if (!is_card_string(cardsSplit[i])) {
            free(cardsSplit);
            deck_destroy(outDeck);
            return false;
        }
        outDeck->cards[i] = to_card(cardsSplit[i]);
    }

    free(cardsSplit);
    outDeck->numCards = numCards;
    return true;
}

// see header
char* msg_encode_hand(Deck deck) {
    // start with code
    char* numStr = int_to_string(deck.numCards);
    // space for number string, comma, each card + its comma
    // and trailing \0
    char* payload = calloc(
            strlen(numStr) + 2 + 3 * deck.numCards, sizeof(char));
    int n = 0;
    // snprintf returns number of non-null bytes written
    n += sprintf(payload + n, "%s", numStr);
    free(numStr);
    
    for (int i = 0; i < deck.numCards; i++) {
        char cardStr[3];
        // note comma in print string
        n += sprintf(payload + n, ",%s", 
                fmt_card(cardStr, deck.cards[i], false));
    }
    return payload;
}

// see header
bool msg_decode_int(char* payload, int* outInt) {
    int result = parse_int(payload);
    if (result >= 0) {
        *outInt = result;
    }
    return result >= 0;
}

// see header
char* msg_encode_int(int value) {
    return int_to_string(value);
}

// see header
bool msg_decode_played(char* payload, PlayedTuple* outTuple) {
    *outTuple = (PlayedTuple) {0};
    char* split[2];
    if (tokenise(payload, ',', split, 2) != 2) {
        DEBUG_PRINT("missing comma");
        return false;
    }
    int player = parse_int(split[0]);
    if (player < 0) {
        DEBUG_PRINT("invalid player number");
        return false;
    }
    if (!is_card_string(split[1])) {
        DEBUG_PRINT("invalid card");
        return false;
    }
    outTuple->player = player;
    outTuple->card = to_card(split[1]);
    return true;
}

// see header
char* msg_encode_played(PlayedTuple tuple) {
    char* playerStr = int_to_string(tuple.player);

    // player number length + 3 for card and comma + \0
    char* payload = calloc(strlen(playerStr) + 4, sizeof(char));
    char cardStr[3];
    sprintf(payload, "%s,%s", playerStr, fmt_card(cardStr, tuple.card, false));
    return payload;
}

// see header
bool msg_decode_card(char* payload, Card* outCard) {
    if (!is_card_string(payload)) {
        return false;
    }
    *outCard = to_card(payload);
    return true;
}

// see header
char* msg_encode_card(Card card) {
    char* payload = calloc(3, sizeof(char));
    return fmt_card(payload, card, false);
}

