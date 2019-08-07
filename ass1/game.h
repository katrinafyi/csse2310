#ifndef GAME_H
#define GAME_H

#include <stdbool.h>

#include "deck.h"
#include "board.h"

#define NUM_PLAYERS 2
#define NUM_HAND 6

/* Struct for all of the game state. Completely describes a game's current
 * state.
 */
typedef struct GameState {
    int currPlayer;
    int numDrawn;
    Card playerHands[NUM_PLAYERS * NUM_HAND];
    char* deckFile;
    Deck* deck;

    BoardState* boardState;
} GameState;

/* Remark:
 * Any function in this file taking a GameState* parameter will update
 * the state struct as appropriate.
 */

/* Loads the given file into the given state struct, filling out all its
 * members.
 * Returns true on success, false on any save file formatting errors.
 */
bool load_game_file(GameState* gameState, char* saveFile);

/* Initialises the game state. Variable length arrays (deckFile, deck,
 * boardState) are set to NULL.
 */
void init_game_state(GameState* gameState);

/* Saves the game state to the given file, overwriting it if it exists.
 * Returns false on any errors saving the file.
 */
bool save_game_file(GameState* gameState, char* saveFile);

/* Deals the intial cards. That is, NUM_HAND-1 to each player sequentially,
 * all p1's, then p2's, etc.
 * Returns false if the deck does not have enough cards.
 */
bool deal_cards(GameState* gameState);

/* Draws a single card from the deck of gameState, returning the drawn card.
 * Updates numDrawn in the state.
 * Returns a null card if there are no more cards.
 */
Card draw_card(GameState* gameState);
/* Executes the main game loop with the given playerTypes.
 * playerTypes should contain NUM_PLAYERS characters,
 * 'h' for human and 'a' for auto players.
 * This function loops over player turns, asking for input and
 * displaying output.
 * Returns an exit code as defined in the spec.
 */
int exec_game_loop(GameState* gameState, char* playerTypes);

/* Prints the cards in the hand of the current player, single space
 * separated with trailing \n.
 * Only prints the cards, not "Hand: " or similar.
 */
void print_hand(GameState* gameState);

/* Plays the current player's turn according to the automatic player
 * algorithm, printing what move was played.
 */
void play_auto_turn(GameState* gameState);

#endif

