#!/bin/bash
# THIS FILE IS A COPIED TEMPLATE! - Only edit in /pkg/src/



cat "$1" |
	awk -v CF="$2" '{ gsub ("@@VERSION@@", CF, $0); print }'
