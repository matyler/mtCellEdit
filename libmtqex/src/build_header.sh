#!/bin/bash
# Build the header file for Qt4 or Qt5
# M.Tyler 2018-12-16


case "$1" in
	"mtqex4.h" )
		HEADER_GUARD=$(cat << EOF
#ifndef MTQEX4_H_
#define MTQEX4_H_
EOF
)
		HEADER_INCLUDE="#include <QtGui>"
		TABLEWIDGET_DEFINE=$(cat << EOF
// tableWidget->horizontalHeader ()->setClickable
#define QEX_SETCLICKABLE	setClickable

// tableWidget->horizontalHeader()->setResizeMode
#define QEX_RESIZEMODE		setResizeMode
EOF
)
		;;

	"mtqex5.h" )
		HEADER_GUARD=$(cat << EOF
#ifndef MTQEX5_H_
#define MTQEX5_H_
EOF
)
		HEADER_INCLUDE="#include <QtWidgets>"
		TABLEWIDGET_DEFINE=$(cat << EOF
// tableWidget->horizontalHeader ()->setSectionsClickable
#define QEX_SETCLICKABLE	setSectionsClickable

// tableWidget->horizontalHeader ()->setSectionResizeMode
#define QEX_RESIZEMODE		setSectionResizeMode
EOF
)
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
		"@@TABLEWIDGET_DEFINE@@" )	LINE="$TABLEWIDGET_DEFINE";;
	esac

	echo "$LINE"
done

