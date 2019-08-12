#!/usr/bin/env zsh

set -e
set +v

tmp="$(mktemp -d --tmpdir=. tmp_style.XXXXXX)"
#tmp="tmp"
cp *.c *.h "$tmp"
echo "$tmp"
cd "$tmp"

#sed -r -i 's|^(\s*)DEBUG_PRINT|\1//disabled:DEBUG_PRINT|g' style_checker/*.c

# redefine DEBUG_ macros to do nothing.
#sed -r -i 's|#define (DEBUG_[A-Z_]+)\(.*|#define \1\(...\) do {} while \(0\)|g' *.c *.h

# delete lines marked // style_deleteme
# sed -r -i 's|^.*// style_deleteme.*$||g' *.c *.h

# deletes the macro from places where its used, leaving a ("asdf%s", ...)
# expression which is valid.
sed -r -i 's|^(\s+)DEBUG(_PRINTF?)|\1|g' *.c

style_output="$(style.sh)"

# ignore spam messages
echo "$style_output" | grep -E -v '(THIS IS NOT|THE STYLE)' | grep -v 'No errors found'

set +e # disable quit on errors.

errors="$(echo "$style_output" | grep 'total errors found')"

compile_errors="$(echo "$style_output" | grep 'does not compile')"
if [[ -n "$compile_errors" ]]; then
    tput setaf 1
    echo
    echo "COMPILE ERRORS"
    tput reset
    echo "$compile_errors"
fi

cd ..
# rm -r "$tmp"

[[ -n "$errors" ]] && exit 1;
