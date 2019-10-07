#!/bin/bash

port="$1"
pid="$2"
if [[ -z "$port" ]] || [[ -z "$pid" ]]; then
    echo "invalid port or pid"
    exit 1
fi

arg=-N
if [[ "$(hostname)" = 'moss.labs.eait.uq.edu.au' ]]; then
    arg=--send-only
fi

for i in {1..100}; do
    nc $arg localhost $port <<EOF &
IM:1000:nc$i
Deliver:1:mat
Withdraw:2:mat
Defer:10:Deliver:1:mat
Withdraw:1:diamond
Execute:10
Deliver:1:mat$i
Transfer:1:mat$i:nc$i
Deliver:1:0mat$i
EOF
    kill -hup $pid
done
