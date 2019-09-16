#!/bin/bash

name=$0
numPlayers=$1
playerNum=$2
threshold=$3
handSize=$4

if [[ -z "$playerNum" ]]; then
    playerNum=0
fi

f="t$playerNum"

echo "$0: $@" >&2
if [[ -f "$f.out" ]]; then
    echo "$0: sending to hub: $f.out" >&2
    cat $f.out
else
    printf '@'
fi

# close stdout
exec 1>&-

echo "$0: recording input to file: $f.in" >&2

tee $f.in >/dev/null
