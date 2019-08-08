#!/bin/bash

! [[ -f './static_tests/testa1.sh' ]] && ./copy_tests.sh

exec ./static_tests/testa1.sh
