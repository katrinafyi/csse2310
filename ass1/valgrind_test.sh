#!/usr/bin/env bash

test_sh='testa1.sh'

! $test_sh > /dev/null 2>&1 && test_sh='_tests/testa1.sh'

if ! $test_sh >/dev/null 2>&1; then
    echo 'test script not found in path or ./_tests'
    exit 1
fi

tests="$($test_sh explain | grep ./bark)"
IFS='
'
i=1
for cmd in $tests; do
    if ! [[ "$cmd" == *'<'* ]]; then
        cmd="$cmd < /dev/null"
    fi
    echo "$cmd"
    log_file="tmp_valgrind_$i"
    eval "valgrind --leak-check=full --log-file=$log_file $cmd";
    echo >> $log_file
    echo "$cmd" >> $log_file
    ((i++))
done;

echo "Opening logs of leaked tests"
grep -L 'no leaks are possible' tmp_valgrind_* | xargs less

echo "Cleaning up files"
rm Assignment*
rm tsave*
rm tmp_valgrind_*
