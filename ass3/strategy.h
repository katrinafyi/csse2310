#ifndef PLAYERINTERFACE_H
#define PLAYERINTERFACE_H

#include "deck.h"
#include "playerState.h"

int strategy_when_leading(PlayerState* playerState);
int strategy_when_following(PlayerState* playerState);

#endif
