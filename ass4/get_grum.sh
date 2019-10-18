#!/bin/bash

scp moss:~uqjfenw1/public/2019/ptesta4/grum.py .
[[ ! -d './tests' ]] && scp -r moss:~/csse2310/ass4/tests . || echo 'tests exists'

