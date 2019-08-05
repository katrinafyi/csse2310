#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include "deck.h"
#include "util.h"

bool load_deck_file(Deck* deck, char* deckFile) {
    deck->numCards = 0; // just in case.
    deck->cards = NULL;
    FILE* file = fopen(deckFile, "r");
    DEBUG_PRINTF("deck file loading: %s\n", deckFile);
    char* numLine;
    if (!safe_read_line(file, &numLine)) {
        return false;
    }
    int numCards = parse_int(numLine);
    if (numCards <= 0) {
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
        deck->cards[i] = to_card(line);
        free(line);
    }
    if (fgetc(file) != EOF) {
        DEBUG_PRINT("junk at end of deck");
        return false;
    }
    fclose(file);
    deck->numCards = numCards;
    return true;
}

bool is_card(char* str) {
    return '1' <= str[0] && str[0] <= '9'
            && 'A' <= str[1] && str[1] <= 'Z';
}

bool is_null_card(Card card) {
    return card.num == 0; // define num == 0 iff card is NULL
}

bool is_blank(char* str) {
    return str[0] == BLANK_CHAR_SAVED && str[1] == BLANK_CHAR_SAVED;
}

Card to_card(char* str) {
    int num = str[0] - '1' + 1;
    char suit = str[1];
    assert(1 <= num && num <= 9 && 'A' <= suit && suit <= 'Z');
    return (Card) {num, suit};
}

char* fmt_card_c(char* str, Card card, char blank) {
    if (is_null_card(card)) {
        snprintf(str, 3, "%c%c", blank, blank);
    } else {
        assert(1 <= card.num && card.num <= 9);
        snprintf(str, 3, "%d%c", card.num, card.suit);
    }
    return str;
}

char* fmt_card(char* str, Card card) {
    return fmt_card_c(str, card, BLANK_CHAR_PRINT);
}
