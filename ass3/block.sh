#!/bin/bash

trap "echo $0: SIGINT 1>&2" SIGINT

printf '@'
while true; do
    sleep 10;
done
