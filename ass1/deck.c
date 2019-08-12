#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include "deck.h"
#include "util.h"

// see header
bool load_deck_file(Deck* deck, char* deckFile) {
    deck->numCards = 0; // just in case we iterate over an errored deck.
    deck->cards = NULL;
    FILE* file = fopen(deckFile, "r");
    DEBUG_PRINTF("deck file loading: %s\n", deckFile);
    char* numLine;
    if (!safe_read_line(file, &numLine)) { // this checks if file is NULL
        DEBUG_PRINT("failed to read number line");
        return false;
    }
    int numCards = parse_int(numLine);
    if (numCards < 0) {
        DEBUG_PRINT("number of cards invalid");
        return false;
    }
    free(numLine);
    // printf("%d cards deteced\n", numCards);
    deck->cards = malloc(sizeof(Card) * numCards);
    for (int i = 0; i < numCards; i++) {
        char* line;
        if (!safe_read_line(file, &line)) {
            return false;
        }
        int valid = strlen(line) == 2 && is_card(line);
        if (!valid) {
            return false;
        }
        deck->cards[i] = to_card(line); // card must be non-null
        free(line);
    }
    if (fgetc(file) != EOF) {
        DEBUG_PRINT("junk at end of deck");
        return false;
    }
    fclose(file);
    deck->numCards = numCards; // only set if we succeeded
    return true;
}

// see header
bool is_card(char* str) {
    return '1' <= str[0] && str[0] <= '9'
            && 'A' <= str[1] && str[1] <= 'Z';
    // these bounds could probably be #define'd
}

// see header
bool is_null_card(Card card) {
    return card.num == 0; // define num == 0 iff card is NULL
}

// see header
bool is_blank(char* str) {
    return str[0] == BLANK_CHAR_SAVED && str[1] == BLANK_CHAR_SAVED;
}

// see header
Card to_card(char* str) {
    int num = str[0] - '1' + 1;
    char suit = str[1];
    assert(1 <= num && num <= 9 && 'A' <= suit && suit <= 'Z');
    return (Card) {num, suit};
}

// see header
char* fmt_card_c(char* str, Card card, char blank) {
    if (is_null_card(card)) {
        str[0] = blank;
        str[1] = blank;
        str[2] = '\0';
    } else {
        assert(1 <= card.num && card.num <= 9);
        str[0] = card.num + '0'; // convert int into character
        str[1] = card.suit;
        str[2] = '\0';
    }
    return str;
}

// see header
char* fmt_card(char* str, Card card) {
    return fmt_card_c(str, card, BLANK_CHAR_PRINT);
}
