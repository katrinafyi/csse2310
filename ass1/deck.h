#ifndef DECK_H
#define DECK_H

#include <stdbool.h>

#define NULL_CARD ((Card) { 0, '~' } )
#define BLANK_CHAR_SAVED '*'
#define BLANK_CHAR_PRINT '.'

typedef struct Card {
    int num;
    char suit;
} Card;

typedef struct Deck {
    int numCards;
    Card* cards;
} Deck;

bool load_deck_file(Deck* deck, char* deckFile);
bool is_card(char* cardStr);
bool is_null_card(Card card);
bool is_blank(char* cardStr);
Card to_card(char* cardStr);
char* fmt_card_c(char* str, Card card, char fillChar);
char* fmt_card(char* str, Card card);

#endif

