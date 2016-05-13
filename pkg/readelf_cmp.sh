#!/bin/sh
# Check installed libs and bins for C language functions used.
# Mark Tyler 2014-3-28


D_BIN=/usr/bin
D_LIB=/usr/lib

F_LIBS="libmtkit.so libmtcelledit.so libmtcedui.so libmtgex.so"
F_BINS="mtcelledit mtcedcli cedutils mtnetlog mtraft"

F_LIBS="$F_LIBS libmtqex4.so"
F_BINS="$F_BINS mtcelledit-qt4 mteleana-qt4 mtraft-qt4"

F_A1=~/ced_api_raw.txt
F_A2=~/ced_api_sorted.txt
F_F1=~/ced_func_raw.txt
F_F2=~/ced_func_sorted.txt
F_D1=~/ced_diff.txt
F_D2=~/ced_diff2.txt
F_G1=~/ced_grep.txt


while [ "$1" != "" ]
do
	case "$1" in
	"flush" )
		rm $F_A1 $F_A2 $F_F1 $F_F2 $F_D1 $F_D2 $F_G1

		echo Removed temp files.
		exit
		;;

	"--bindir" )
		shift
		D_BIN="$1"
		;;

	"--libdir" )
		shift
		D_LIB="$1"
		;;

	* )
		echo Unknown arg "$1"
		exit
		;;
	esac

	shift
done



# Funcs exposed via libs

> $F_A1

for PKG in $F_LIBS
do
	echo $D_LIB/$PKG

	readelf -Ws $D_LIB/$PKG | awk '$5=="GLOBAL" && $7=="11" { print $8 }' | sort -u >> $F_A1
done

cat $F_A1 | sort -u > $F_A2


# Funcs used by bins & libs

> $F_F1

for PKG in $F_BINS
do
	echo $D_BIN/$PKG

	readelf -Ws $D_BIN/$PKG | awk '$5=="GLOBAL" && $7=="UND" { print $8 }' | sort -u >> $F_F1
done

for PKG in $F_LIBS
do
	echo $D_LIB/$PKG

	readelf -Ws $D_LIB/$PKG | awk '$5=="GLOBAL" && $7=="UND" { print $8 }' | sort -u >> $F_F1
done

cat $F_F1 | sort -u > $F_F2


diff -Nurp $F_A2 $F_F2 > $F_D1
cat $F_D1 | awk '/^-/ && !/^--/' | sed 's/^-//' > $F_D2

# Function names in this final list are not used other than by the lib they are defined in.
# They could be:
#  1. Private functions used by the lib (consider making static or put into private.h).
#  2. Cruft - nothing uses them so they can be safely removed.
#
# Go to root of mtCellEdit suite and find out:

cd ..

FUNCS=$(cat $F_D2 )

> $F_G1

for FN in $FUNCS
do
	echo >> $F_G1
	echo $FN >> $F_G1
	echo >> $F_G1

	grep -nr "$FN" . >> $F_G1
done

less $F_G1

