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
    static char* typeStrings[6];
    typeStrings[MSG_HAND] = "HAND";
    typeStrings[MSG_NEW_ROUND] = "NEWROUND";
    typeStrings[MSG_PLAYED_CARD] = "PLAYED";
    typeStrings[MSG_GAME_OVER] = "GAMEOVER";
    typeStrings[MSG_PLAY_CARD] = "PLAY";
    typeStrings[MSG_NULL] = "NULL"; // internal use only

    assert(0 <= type && type < 6);
    return typeStrings[type];
}

// see header
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

// see header
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
        noop_print("read pipe is at EOF");
        return MS_EOF;
    }
    noop_printf("received: %s\n", line);

    MessageType type = MSG_NULL;
    char* payload = NULL;
    for (int t = 0; t < NUM_MESSAGE_TYPES; t++) {
        char* code = msg_code(t); // get string code expected for this type

        // warning: PLAY and PLAYED have a common prefix
        // so played must be checked first and we must break on match.
        if (strncmp(line, code, strlen(code)) == 0) {
            type = t;
            payload = line + strlen(code);
            break;
        }
    }
    if (type == MSG_NULL) {
        noop_print("no matched code");
        free(line);
        return MS_INVALID;
    }

    if (!msg_payload_decode(type, payload, &message.data)) {
        noop_print("invalid payload");
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
    char* payload = msg_payload_encode(message);

    errno = 0;
    int ret = fprintf(file, "%s%s\n", msg_code(message.type), payload);
    noop_printf("sending: %s%s\n", msg_code(message.type), payload);
    fflush(file);
    // DEBUG_PRINTF("errno: %d\n", errno);
    free(payload);

    // only (reasonable) error is EOF caused by broken pipe
    return (ret >= 0) ? MS_OK : MS_EOF;
}

// see header
bool msg_decode_hand(char* payload, Deck* outDeck) {
    char* firstSplit[2];
    // first, split into number of cards and the rest.
    if (tokenise(payload, ',', firstSplit, 2) != 2) {
        noop_print("couldn't find number of cards");
        return false;
    }
    int numCards = parse_int(firstSplit[0]);
    if (numCards <= 0) {
        noop_print("invalid num cards value");
        return false;
    }
    deck_init_empty(outDeck, numCards);

    // split on each comma and parse cards
    char** cardsSplit = calloc(numCards, sizeof(char*));
    if (tokenise(firstSplit[1], ',', cardsSplit, numCards) != numCards) {
        noop_print("wrong number of cards");
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
    char* numPart = int_to_string(deck.numCards);
    // space for number string, comma, each card + its comma
    // and trailing \0
    char* payload = calloc(
            strlen(numPart) + 2 + 3 * deck.numCards, sizeof(char));
    int n = 0;
    // snprintf returns number of non-null bytes written
    n += sprintf(payload + n, "%s", numPart);
    free(numPart);
    
    for (int i = 0; i < deck.numCards; i++) {
        char cardBuf[3];
        // note comma in print string
        n += sprintf(payload + n, ",%s", 
                fmt_card(cardBuf, deck.cards[i], false));
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
        noop_print("missing comma");
        return false;
    }
    int player = parse_int(split[0]);
    if (player < 0) {
        noop_print("invalid player number");
        return false;
    }
    if (!is_card_string(split[1])) {
        noop_print("invalid card");
        return false;
    }
    outTuple->player = player;
    outTuple->card = to_card(split[1]);
    return true;
}

// see header
char* msg_encode_played(PlayedTuple tuple) {
    char* playerPart = int_to_string(tuple.player);

    // player number length + 3 for card and comma + \0
    char* payload = calloc(strlen(playerPart) + 4, sizeof(char));
    char str[3];
    sprintf(payload, "%s,%s", playerPart, fmt_card(str, tuple.card, false));

    free(playerPart);
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

// see header
Message msg_hand(Deck hand) {
    Message message = {0};
    message.type = MSG_HAND;
    message.data.hand = hand;
    return message;
}

// see header
Message msg_new_round(int leadPlayer) {
    Message message = {0};
    message.type = MSG_NEW_ROUND;
    message.data.leadPlayer = leadPlayer;
    return message;
}

// see header
Message msg_played_card(int player, Card card) {
    Message message = {0};
    message.type = MSG_PLAYED_CARD;
    message.data.playedTuple = (PlayedTuple) {player, card};
    return message;
}

// see header
Message msg_game_over(void) {
    Message message = {0};
    message.type = MSG_GAME_OVER;
    return message;
}

// see header
Message msg_play_card(Card card) {
    Message message = {0};
    message.type = MSG_PLAY_CARD;
    message.data.card = card;
    return message;
}
