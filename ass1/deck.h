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
    // rank of the card
    int num;
    // suit of the card
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

/* Returns a new empty deck initialised appropriately.
 */
Deck new_deck(void);

/* Frees memory associated with this deck.
 */
void destroy_deck(Deck* deck);

/* Returns true if str points to a valid non-blank card.
 * str need not be null-terminated but must have at least 2 characters.
 */
bool is_card(char* str);

/* Returns true if the card struct is a null value.
 * That is, it should be treated as a blank or missing card.
 */
bool is_null_card(Card card);
// yes, these 2 functions are probably named too similarly.

/* Returns true if str points to a blank card. That is, if it points to
 * two BLANK_CHAR_SAVED characters.
 */
bool is_blank(char* str);

/* Parses the given string into a Card struct, returning the struct.
 * str must be a non-null card.
 */
Card to_card(char* str);

/* Formats the given card into the given string, replacing null cards with
 * the given blank character.
 * str contain space for at least 3 characters (including \0).
 */
char* fmt_card_c(char* str, Card card, char blank);

/* Formats the given card into the given string, replacing null cards with
 * 2 of BLANK_CHAR_PRINT and \0 terminating the string.
 */
char* fmt_card(char* str, Card card);

#endif

