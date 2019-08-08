#!/bin/bash

if [ $# -lt 1 ]
then
    args=test
else
    args=$@
fi

if [ -d .git ]
then
    echo "It appears that you are trying to use git for this project."
    echo "We do not encourage this."
    exit 1
fi

if [ -d .hg ]
then
    echo "While your independence of thought in not following the git"
    echo "herd is appreciated, you know very well that we are not"
    echo "using hg for this project."
    exit 1
fi

if [ -d .jazz* ]
then
    echo -n "This behaviour is not rational..."
    sleep 1
    echo "here is a train"
    sleep 1
    sl
    exit 1
fi

if [ -d CVS ]
then
    echo "No! Just no."
    exit 1
fi

svn info >/dev/null 2>/dev/null
sstat=$?
if [ $sstat != 0 ]
then
    echo "It looks like you might not be running this from an svn working"
    echo "directory. Since the use of subversion is both assessable in "
    echo "the course and necessary for assignment submission, we suggest"
    echo "fixing the subversion issue first."
    exit 1
fi

d="$(dirname $0)"; PYTHONPATH="$d/lib:$PYTHONPATH" "$d"/tests/grum.py $args

