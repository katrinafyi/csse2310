#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <ctype.h>

#include "deck.h"
#include "util.h"

// see header
void deck_destroy(Deck* deck) {
    if (deck->cards != NULL) {
        free(deck->cards);
        deck->cards = NULL;
    }
}

/* Actually loads the deck file into the given deck. Called by wrapper
 * function which manages memory. Returns true if deck is valid and loaded.
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
            noop_print("read line failed");
            return false;
        }
        if (strlen(line) != 2 || !is_card(line)) {
            noop_printf("invalid card: |%s|\n", line);
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
void deck_init_empty(Deck* deck, int numCards) {
    deck->numCards = numCards;
    deck->cards = calloc(numCards, sizeof(Card));
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
    // if false, clear any allocated memory.
    if (!ret && deck->cards != NULL) {
        free(deck->cards);
        deck->cards = NULL;
    }
    return ret;
}

// see header
void deck_clear(Deck* deck) {
    for (int i = 0; i < deck->numCards; i++) {
        deck->cards[i] = NULL_CARD;
    }
}

// see header
bool deck_is_full(Deck* deck) {
    for (int i = 0; i < deck->numCards; i++) {
        if (is_null_card(deck->cards[i])) {
            return false;
        }
    }
    return true;
}

// see header
bool deck_is_empty(Deck* deck) {
    for (int i = 0; i < deck->numCards; i++) {
        if (!is_null_card(deck->cards[i])) {
            return false;
        }
    }
    return true;
}


// see header
int deck_search(Deck* hand, char* order, bool high) {
    int len = strlen(order);
    for (int s = 0; s < len; s++) {
        char suit = order[s];
        int index = deck_best_card(hand, suit, high);
        if (index != -1) {
            return index;
        }
    }
    return -1;
}

// see header
int deck_best_card(Deck* deck, char suit, bool high) {
    Card best = NULL_CARD;
    int bestIndex = -1;
    // multiplier to change direction of optimality.
    int mult = high ? 1 : -1;
    for (int i = 0; i < deck->numCards; i++) {
        if (deck->cards[i].suit != suit) {
            continue;
        }
        if (is_null_card(best) || 
                deck->cards[i].rank * mult > best.rank * mult) {
            best = deck->cards[i];
            bestIndex = i;
        }
    }
    return bestIndex;
}

// see header
int deck_index_of(Deck* deck, Card card) {
    for (int i = 0; i < deck->numCards; i++) {
        if (cards_equal(card, deck->cards[i])) {
            return i;
        }
    }
    return -1;
}

// see header
void deck_remove_card(Deck* deck, Card card) {
    int index = deck_index_of(deck, card);
    assert(index >= 0);
    deck->cards[index] = NULL_CARD;
}

// see header
bool cards_equal(Card card1, Card card2) {
    return card1.suit == card2.suit && card1.rank == card2.rank;
}

// see header
bool is_card(char* str) {
    switch (str[0]) {
        case 'S':
        case 'C':
        case 'D':
        case 'H':
            break;
        default:
            return false;
    }
    // any lowercase hexadecimal digit except 0
    return !isupper(str[1]) && isxdigit(str[1]) && str[1] != '0';
}

// see header
bool is_card_string(char* str) {
    return strlen(str) == 2 && is_card(str);
}

// see header
bool is_null_card(Card card) {
    return cards_equal(NULL_CARD, card);
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

