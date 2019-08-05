#ifndef DECK_H
#define DECK_H

#include <stdbool.h>

#define NULL_CARD ((Card) { 0, '~' } )
#define BLANK_CHAR_SAVED '*'
#define BLANK_CHAR_PRINT '.'

/* Card with an integer `num` (its rank) and a character `suit`.
 * Assumptions:
 * 0 <= num <= 9
 * 'A' <= suit <= 'Z'
 */
typedef struct Card {
    int num;
    char suit;
} Card;

/* Deck storing the total number of cards in the deck and a pointer to a
 * Card array. This struct can be filled from just reading the deck file.
 * Assumes cards contains sufficient space for the given numCards.
 */
typedef struct Deck {
    int numCards;
    Card* cards;
} Deck;

/* Loads the given deckFile into the given deck struct.
 * Returns true on success, false for any deck file formatting errors.
 * File structure assumptions are described in the implementation.
 */
bool load_deck_file(Deck* deck, char* deckFile);
/* Returns true if cardStr points to a valid non-blank card.
 * cardStr need not be null-terminated but must have >= 2 characters.
 */
bool is_card(char* cardStr);
/* Returns true if the card struct is a null value.
 * That is, it should be treated as a blank or missing card.
 */
bool is_null_card(Card card);
/* Returns true if cardStr points to a blank card. That is, if it points to
 * two BLANK_CHAR_SAVED characters.
 */
bool is_blank(char* cardStr);
/* Parses the given string into a Card struct, returning the struct.
 * cardStr must be a non-null card.
 */
Card to_card(char* cardStr);
/* Formats the given card into the given string, replacing null cards with
 * the given fillChar.
 * str contain space for at least 3 characters.
 */
char* fmt_card_c(char* str, Card card, char fillChar);
/* Formats the given card into the given string, replacing null cards with
 * 2 of BLANK_CHAR_PRINT.
 */
char* fmt_card(char* str, Card card);

#endif

