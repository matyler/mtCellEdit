#!/bin/bash
# THIS FILE IS A COPIED TEMPLATE! - Only edit in /pkg/src/


if [ "$3" = "" ]
then
	BIN_NAME=$(echo "$1" | sed 's/\.t2t$//')
else
	BIN_NAME="$3"
fi


BIN_UPPER=$(echo "$BIN_NAME" | tr '[a-z]' '[A-Z]')


# HEADER
cat << EOF
$BIN_UPPER
Version $2
%%date(%Y-%m-%d)

%!encoding: utf-8

EOF


# BODY
cat "$1" | awk -v CF="$BIN_NAME" '{ gsub ("@BIN_NAME@", CF, $0); print }'


# FOOTER
cat << EOF

= HOMEPAGE =
http://ced.marktyler.org/


= AUTHOR =
Mark Tyler

EOF
