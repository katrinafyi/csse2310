#!/usr/bin/env zsh

set -e

tmp="$(mktemp -d)"
#tmp="tmp"
cp *.c *.h "$tmp"
echo "$tmp"
cd "$tmp"


#sed -r -i 's|^(\s*)DEBUG_PRINT|\1//disabled:DEBUG_PRINT|g' style_checker/*.c
echo "$files"
# redefine DEBUG_ macros to do nothing.
sed -r -i 's|#define (DEBUG_[A-Z_]+)\(.*|#define \1\(...\) do {} while \(0\)|g' *.c *.h

# delete lines marked // style_deleteme
sed -r -i 's|^.*// style_deleteme.*$||g' *.c *.h

# you know what? just delete all DEBUG_ statements
sed -r -i 's|(DEBUG_PRINTF?)|//debug:|g' *.c

style.sh | grep -E -v '(THIS IS NOT|THE STYLE)' | grep -v 'No errors found'

rm -r "$tmp"
