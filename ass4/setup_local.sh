#!/usr/bin/env bash

set -e

boost="$(find /usr/lib -name 'libboost_python-py2*.so*1.53.0')"
if [[ -z "$boost" ]]; then
    echo "could not find libboost_python-py2*.so on your system"
    echo "install libboost-python-dev or similar"
    exit 1
fi

echo "linking boost library"
#cd "$(dirname $0)/_tests"
#cp -vf $boost ./lib/libboost_python.so.1.53.0
ln -sf $boost ./libboost_python.so.1.53.0

echo 'cleaning existing .py files'
rm -vf marks/*.py

echo "decompiling marks python files"
uncompyle6 -o ./marks ./marks/*.pyc

echo "patching runner.py"
runner=./marks/runner.py
suite=./marks/suite.py
sed -i '/^#/d' $runner
echo "386d6ee6872d9643085ea384c4eb3720 $runner
bcf945e8bbbe473d3425c1a88f39b28b $suite" | md5sum -c
sed -i '35d' $runner
sed -E -i 's/(not \(result\.module_setup_failed\(module_name\) or result\.class_setup_failed\(test\.__class__\)\))/True or \1/' $suite

echo "moving pyc files"
mkdir -p marks/pyc
mv -v marks/*.pyc marks/pyc
