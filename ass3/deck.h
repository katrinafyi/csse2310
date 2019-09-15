#ifndef DECK_H
#define DECK_H

#include <stdbool.h>

#define NULL_CARD ((Card) { '~', 0 } )

/* Card with an integer `num` (its rank) and a character `suit`.
 * Assumptions:
 * 0 <= rank <= 16
 * suit is 'A', 'D', 'S' or 'C'
 * 0 rank indicates null card.
 */
typedef struct Card {
    // suit of the card
    char suit;
    // rank of the card
    int rank;
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
bool deck_init_file(Deck* deck, char* deckFile);

/* Initialises an empty deck with the given number of cards.
 */
void deck_init_empty(Deck* deck, int numCards);

/* Frees memory associated with this deck.
 */
void deck_destroy(Deck* deck);

/* Clears all cards from the given deck, replace them with NULL_CARD. */
void deck_clear(Deck* deck);

/* Returns true if all slots in the deck are non-null. */
bool deck_is_full(Deck* deck);

/* Returns the index of the highest or lowest card of the given suit in hand, 
 * or a negative if there is no such card.
 *
 * If high is true, returns the highest card, otherwise returns lowest.
 */
int deck_best_card_index(Deck* hand, char suit, bool high);

/* Returns the index of the given card in the given deck, or -1 if it does
 * not appear.
 */
int deck_index_of(Deck* hand, Card card);

/* Removes the given card from the given hand, replacing it with null. 
 * The card SHOULD exist in the hand.
 */
void deck_remove_card(Deck* hand, Card card);

/* Returns true if the given cards are equal. That is, their suit and rank
 * are both the same.
 */
bool cards_equal(Card card1, Card card2);

/* Returns true if str points to a valid non-blank card.
 * str need not be null-terminated but must have at least 2 characters.
 * Suits should be a valid uppercase suit.
 * Ranks are interpreted as hexadecimal and must be lowercase.
 *
 * Note that NULL_CARD is intentionally not a valid card by this definition.
 */
bool is_card(char* str);

/* Returns true if the card struct is a null value.
 * That is, it should be treated as a blank or missing card.
 */
bool is_null_card(Card card);
// yes, these 2 functions are probably named too similarly.

/* Parses the given string into a Card struct, returning the struct.
 * str must be a non-blank card.
 */
Card to_card(char* str);

/* Formats the given card into the given string, optionally putting a '.'
 * between the suit and rank. Returns the string.
 * str contain space for at least 3 characters (including \0) or 4 characters
 * if dotSeparated.
 */
char* fmt_card(char* str, Card card, bool dotSeparated);

#endif

