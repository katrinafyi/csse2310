#ifndef PLAYERINTERFACE_H
#define PLAYERINTERFACE_H

#include "deck.h"
#include "playerState.h"

// this is an interface (in the C# sense), to be implemented differently
// by each player

/* Returns the index of a card to play when the player IS the leader,
 * with the current state given by playerState.
 */
int strategy_when_leading(PlayerState* playerState);

/* Returns the index of a card to play when the current player is NOT the
 * leader, with the current state given by playerState.
 */
int strategy_when_following(PlayerState* playerState);

#endif
