#ifndef GAME_H
#define GAME_H

#include <stdbool.h>

#include "deck.h"
#include "board.h"

// theoretically, game can adapt to any number of players.
#define NUM_PLAYERS 2
// this is the number of cards in the CURRENT player's hand, so we can
// allocate arrays without using +1 all the time.
#define NUM_HAND 6

/* Struct for all of the game state. Completely describes a game's current
 * state.
 *
 * Note: deckFile must be set to a malloc'd string.
 */
typedef struct GameState {
    // 0-based index of the current player
    int currPlayer;
    // number of cards already drawn from the deck
    int numDrawn;
    // array for player hands
    Card playerHands[NUM_PLAYERS * NUM_HAND];
    // path of the used deck file. should be a malloc'd string
    char* deckFile;
    // pointer to the deck
    Deck* deck;
    // pointer to the board
    BoardState* boardState;
} GameState;

/* Remark:
 * Any function in this file taking a GameState* parameter will update
 * the state struct as appropriate.
 */

/* These initialisation functions are a mess. TODO for next time, actually use
 * proper create/destroy patterns, appropriately encapsulate scope of each
 * struct and use opaque structs.
 *
 * Unfortunately, the process of the game's state is somewhat
 * convoluted. Ideally, I want to split the exit cases into individual
 * functions which fail for exactly one reason, not multiple possible exit
 * codes.
 *
 * 1. init
 * 2. read arguments and player types, determine if loading save or new game
 *    EXIT if player types are wrong.
 * 
 * 3. if new game, malloc empty board with dimensions given and initialise
 *    game state to starting values. EXIT if dimensions wrong.
 * 
 * 4. if loading save, parse save file and EXIT if savefile errors occur.
 * 5. during this process, we need to malloc the board. in general, 
 *    game state will bootstrap init of the board.
 * 
 * 6. parse deck file, EXIT if deck file format wrong (yes, this is in the
 *    wrong order compared to spec, the spec is dumb).
 * 
 * 7. if new game and < 11 cards, EXIT short deck. draw 10 cards.
 * 8. if loading save and board full, EXIT board full.
 *
 * 9. start main game, EXIT eof on ^D or normally on game finish.
 * 10. end
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

/* Returns a new game state struct with values safely initialised.
 */
GameState new_game(void);

/* Clears all allocated memory from game state and calls destroy on deck
 * and board as well. gameState and its sub-structs should absolutely not be
 * used afterwards.
 */
void destroy_game(GameState* gameState);

/* Saves the game state to the given file, overwriting it if it exists.
 * Returns false on any errors saving the file.
 */
bool save_game_file(GameState* gameState, char* saveFile);

/* Deals the intial cards. That is, NUM_HAND-1 to each player sequentially.
 * (all p1's, then p2's, etc.)
 * Returns false if the deck does not have at least (NUM_HAND-1)*NUM_PLAYERS+1
 * cards (enough for initial dealing, then 1 for first player).
 */
bool deal_cards(GameState* gameState);

/* Draws a single card from the deck of gameState, returning the drawn card.
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

/* Prints the cards in the hand of the given player, separated by the given
 * sep string to the given file.
 * Only prints the cards, not "Hand: " or similar. Includes trailing \n.
 * Returns true on success, false otherwise.
 */
bool fprint_hand(GameState* gameState, FILE* file, char* sep, int player);

/* Prints the current player's hand to stdout, single space separated.
 * Equivalent to
 * fprint_hand(gameState, stdout, " ");
 */
void print_hand(GameState* gameState);

/* Plays the current player's turn according to the automatic player
 * algorithm, printing what move was played.
 */
void play_auto_turn(GameState* gameState);

#endif

