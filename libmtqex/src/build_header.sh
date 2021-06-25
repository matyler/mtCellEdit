#!/bin/bash
# Build the header file for Qt5 or Qt6
# M.Tyler 2018-12-16 & 2020-12-16


case "$1" in
	"mtqex5.h" )
		HEADER_GUARD=$(cat << EOF
#ifndef MTQEX5_H_
#define MTQEX5_H_
EOF
)
		HEADER_INCLUDE="#include <QtWidgets>"
		;;

	"mtqex6.h" )
		HEADER_GUARD=$(cat << EOF
#ifndef MTQEX6_H_
#define MTQEX6_H_
EOF
)
		HEADER_INCLUDE="#include <QtWidgets>"
		;;

	* )	echo "Bad argument: '$1'"
		echo
		exit 1
		;;
esac


while IFS= read LINE
do
	case "$LINE" in
		"@@HEADER_GUARD@@" )		LINE="$HEADER_GUARD";;
		"@@HEADER_INCLUDE@@" )		LINE="$HEADER_INCLUDE";;
	esac

	echo "$LINE"
done

