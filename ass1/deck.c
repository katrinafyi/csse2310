#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "deck.h"
#include "util.h"

bool load_deck_file(Deck* deck, char* deckFile) {
    deck->numCards = 0; // just in case.
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
    // printf("%d cards deteced\n", numCards);
    deck->cards = malloc(sizeof(Card)*numCards);
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

bool is_card(char* cardStr) {
    return '1' <= *cardStr && *cardStr <= '9'
        && 'A' <= *(cardStr+1) && *(cardStr+1) <= 'Z';
}

bool is_blank(char* cardStr) {
    return cardStr[0] == BLANK_CHAR_SAVED && cardStr[1] == BLANK_CHAR_SAVED;
}

Card to_card(char* cardStr) {
    return (Card) { *cardStr-'1'+1, *(cardStr+1) };
}

char* fmt_card_c(Card card, char fillChar) {
    char* str = malloc(sizeof(char)*3);
    if (card.num == 0) {
        sprintf(str, "%c%c", fillChar, fillChar);
    } else {
        sprintf(str, "%d%c", card.num, card.suit);
    }
    return str;
}

char* fmt_card(Card card) {
    return fmt_card_c(card, BLANK_CHAR_PRINT);
}
