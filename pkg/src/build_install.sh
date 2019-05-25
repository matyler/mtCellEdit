#!/bin/bash
# Build and install script for all packages.
# Mark Tyler 2011-12-7


. ./src/build_generic.sh
. ./src/_build_global.txt


MT_STARTUP

MT_PARSE_ARGS "$@"

MT_CREATE_BCFILE

MT_DISPLAY_INFO "Generic - ./configure, make, make install"

MT_ACTION_BUILD

