#!/bin/sh



cat "$1" |
	awk -v CF="$2" '{ gsub ("@BIN_INSTALL@", CF, $0); print }'	|
	awk -v CF="$3" '{ gsub ("@BIN_NAME@", CF, $0); print }'		|
	awk -v CF="$4" '{ gsub ("@APP_NAME@", CF, $0); print }'		|
	awk -v CF="$5" '{ gsub ("@BIN_SUFFIX@", CF, $0); print }'
