#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include "deck.h"
#include "util.h"

// see header
Deck new_deck(void) {
    Deck deck;
    deck.numCards = 0;
    deck.cards = NULL;
    return deck;
}

// see header
void destroy_deck(Deck* deck) {
    free(deck->cards);
    deck->cards = NULL;
}

/* Actually loads the deck file into the given deck. Called by wrapper
 * function which manages memory.
 */
bool do_load_deck(Deck* deck, FILE* file) {
    char* numLine = NULL;
    if (!safe_read_line(file, &numLine)) { // this checks if file is NULL
        noop_print("failed to read number line");
        return false;
    }
    int numCards = parse_int(numLine);
    if (numCards < 0) {
        noop_print("number of cards invalid");
        free(numLine);
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
        if (strlen(line) != 2 || !is_card(line)) {
            free(line);
            return false;
        }
        deck->cards[i] = to_card(line); // card must be non-null
        free(line);
    }
    if (fgetc(file) != EOF) {
        noop_print("junk at end of deck");
        return false;
    }
    deck->numCards = numCards; // only set if we succeeded
    return true;
}

// see header
bool deck_init_file(Deck* deck, char* deckFile) {
    deck->numCards = 0; // just in case we iterate over an errored deck.
    deck->cards = NULL;

    noop_printf("deck file loading: %s\n", deckFile);
    FILE* file = fopen(deckFile, "r");
    if (file == NULL) {
        return false;
    }

    bool ret = do_load_deck(deck, file);
    fclose(file);
    return ret;
}

// see header
bool is_card(char* str) {
    // these bounds could probably be #define'd
    switch (str[1]) {
        case 'A':
        case 'C':
        case 'D':
        case 'H':
            break;
        default:
            return false;
    }
    return !isupper(str[1]) && isxdigit(str[1]) && str[1] != '0';
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
    assert(is_card(str));
    int rank;
    if (!isdigit(str[1])) {
        // rank is a lowercase hex char
        rank = str[1] - 'a' + 10;
    } else {
        // rank is a digit
        rank = str[1] - '0';
    }
    char suit = str[0];
    return (Card) {suit, rank};
}

// see header
char* fmt_card(char* str, Card card, bool dotSeparated) {
    assert(card.rank < 16);
    if (dotSeparated) {
        snprintf(str, 4, "%c.%x", card.suit, card.rank);
    } else {
        snprintf(str, 3, "%c%x", card.suit, card.rank);
    }
    return str;
}

