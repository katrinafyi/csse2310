#!/usr/bin/env bash

tests="$(testa1.sh explain | grep ./bark)"
IFS='
'
i=1
for cmd in $tests; do
    if ! [[ "$cmd" == *'<'* ]]; then
        cmd="$cmd < /dev/null"
    fi
    echo "$cmd"
    log_file="tmp_valgrind_$i"
    echo "$cmd" > $log_file
    eval "valgrind --leak-check=full --log-file=$log_file $cmd";
    ((i++))
done;

echo "Opening logs of leaked tests"
grep -L 'no leaks are possible' tmp_valgrind_* | xargs less

echo "Cleaning up files"
rm Assignment*
rm tmp_valgrind_*
