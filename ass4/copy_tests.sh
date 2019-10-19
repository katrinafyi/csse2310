#!/usr/bin/env zsh

n=4
test_sh=testa$n.sh

if ! $test_sh --help > /dev/null 2>&1; then
    echo "$test_sh failed in current directory."
    exit 1
fi


strace="$(strace -f $test_sh explain 2>&1 1>/dev/null | \
    grep -v ENOENT | grep 'open(' | grep /marks/ | \
    sed -E 's/^.*"([^"]+)".*$/\1/')"

svn_url=s"$(svn info | grep '^URL' | awk '{print $NF}')"
if [[ -z "$svn_url" ]]; then
    echo "getting svn url failed."
    exit 2
fi



mkdir -p '_tests'
cd _tests

cp -rvL ../tests .
cp -v `which $test_sh` .

mkdir -p ./lib/marks/pyc
py_libs=("${(f)strace}")
for lib in $py_libs; do
    cp -v $lib ./lib/marks/pyc
done

sed -i 's|~uqjfenw1/public/2019/ptesta'$n'/grum.py|d="$(dirname $0)"; LD_LIBRARY_PATH="$d/lib:$LD_LIBRARY_PATH" PYTHONPATH="$d/lib:$PYTHONPATH" "$d"/tests/grum.py|' $test_sh


