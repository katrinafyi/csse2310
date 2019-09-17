#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "deck.h"

// constructs a test deck, returning test deck.
Deck make_deck(void) {
    Deck d;
    d.numCards = 6;
    d.cards = calloc(6, sizeof(Card)); // leak!
    d.cards[0] = (Card) {'A', 15};
    d.cards[1] = (Card) {'A', 14};
    d.cards[2] = (Card) {'A', 1};
    d.cards[3] = (Card) {'D', 15};
    d.cards[4] = (Card) {'H', 12};
    d.cards[5] = (Card) {'H', 2};
    return d;
}

// tests deck functions
int main(int argc, char** argv) {
    Deck deck = make_deck();

    // test optimal card selection
    assert(deck_best_card(&deck, 'A', true) == 0);
    assert(deck_best_card(&deck, 'A', false) == 2);
    assert(deck_best_card(&deck, 'S', false) == -1);
    assert(deck_best_card(&deck, 'S', true) == -1);
    assert(deck_best_card(&deck, 'D', true) == 3);

    // test index of
    assert(deck_index_of(&deck, (Card) {'A', 14}) == 1);
    assert(deck_index_of(&deck, (Card) {'D', 15}) == 3);
    assert(deck_index_of(&deck, (Card) {'D', 1}) == -1);
}
