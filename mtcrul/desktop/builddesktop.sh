#!/bin/bash
# THIS FILE IS A COPIED TEMPLATE! - Only edit in /pkg/src/



cat "$1" |
	awk -v CF="$2" '{ gsub ("@BIN_INSTALL@", CF, $0); print }'	|
	awk -v CF="$3" '{ gsub ("@BIN_NAME@", CF, $0); print }'		|
	awk -v CF="$4" '{ gsub ("@APP_NAME@", CF, $0); print }'		|
	awk -v CF="$5" '{ gsub ("@BIN_SUFFIX@", CF, $0); print }'


case $(inkscape --version) in

	"Inkscape 1"*)
		PNG_OPT="--export-filename"
		;;
	*)
		PNG_OPT="-z --export-png"
		;;
esac


for SIZE in 16 32 48 64 256
do
	inkscape $PNG_OPT=$SIZE.png --export-width=$SIZE --export-height=$SIZE \
		svg.svg > /dev/null
done

