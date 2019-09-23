#!/bin/bash

trap "echo $0: SIGINT 1>&2; exit" SIGINT

printf '@'
while true; do
    sleep 0.5;
done
