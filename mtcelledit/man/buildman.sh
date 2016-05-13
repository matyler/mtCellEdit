#!/bin/sh


BIN_UPPER=$(echo "$3" | tr '[a-z]' '[A-Z]')


cat "$1" |
	awk -v CF="$2" '{ gsub ("@APP_VERSION@", CF, $0); print }'	|
	awk -v CF="$3" '{ gsub ("@BIN_NAME@", CF, $0); print }'		|
	awk -v CF="$BIN_UPPER" '{ gsub ("@BIN_UPPER@", CF, $0); print }'
