#!/bin/bash

if [[ $# == 0 ]]; then
    echo "$0: no arguments given"
    exit 2
fi

[[ "$DEBUG" == "1" ]] && debug_define=1 || debug_define=0
grep -q '^\s\+DEBUG_PRINT' "$@" >/dev/null 2>&1 && debug_print=1 || debug_print=0

if [[ $debug_print != $debug_define ]]; then
    tput setaf 3
    printf 'error: '
    if [[ $debug_print == 1 ]]; then
        echo "debug prints are enabled but DEBUG is undefined."
    else
        echo "DEBUG is defined but debug prints are not enabled."
    fi
    tput reset
    exit 1
fi
