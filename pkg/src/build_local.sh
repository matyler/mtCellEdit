#!/bin/bash
# Build and install script for all packages to install locally.
# Mark Tyler 2011-12-7 & 2017-9-18


. ./src/build_generic.sh

SUDO=""
#MAKE_ARGS="-j 8"

. ./src/_build_global.txt


MT_STARTUP

MT_PARSE_ARGS "$@"

MT_CREATE_BCFILE

MT_DISPLAY_INFO "Local - ./configure, make, make install"

MT_ACTION_BUILD

