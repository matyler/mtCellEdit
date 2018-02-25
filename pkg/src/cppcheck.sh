#!/bin/sh
# Build script for all packages using cppcheck static analyzer.
# Mark Tyler 2013-2-1


# On error exit
#set -e

cppcheck --enable=all --inconclusive ..

