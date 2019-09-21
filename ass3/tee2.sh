#!/bin/bash

player_name="$0"
player_name="${player_name/tee_/}"
player_num=$2

echo "$0: args: $@" >&2

if [[ -z "$2" ]]; then
    echo "$0: invalid arguments"
    exit 1
fi

tee "$player_name".$2.in | ./$player_name "$@" | tee $player_name.$2.out
