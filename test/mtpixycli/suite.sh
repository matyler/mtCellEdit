#!/bin/bash

. ../mtTest.txt


SHEXE=scripts
CEDCOM="mtpixycli -seed 123 -q -t -patterns /usr/share/mtpixy-qt4/patterns/default.png -shapes /usr/share/mtpixy-qt4/shapes/default.png"


run_sh_head()
{
	echo ----------- $FILE_SH -----------
	echo ----------- $FILE_SH ----------- >> $1
	echo >> $1
}

run_sh_tail()
{
	echo
	echo >> $1
	echo >> $1
}

run_sh_cli()
{
	while [ -n "$1" ]
	do
		FILE_SH=$SHEXE/$1.txt

		case "$VALG" in
		"T" )
			;&
		"Y" )
			run_sh_head $VALG_LOGFILE
			$PROGPREFIX $CEDCOM < $FILE_SH >> $VALG_LOGFILE 2>&1
			run_sh_tail $VALG_LOGFILE
			;;
		esac

		run_sh_head $LOGFILE
		$CEDCOM < $FILE_SH >> $LOGFILE 2>&1
		run_sh_tail $LOGFILE

		shift
	done
}


LOGFILE=output/log.txt
> $LOGFILE


# Creative
run_sh_cli 11 12 13


# Misc, including errors
run_sh_cli 51




valg_results


# Check results
cmp_diff "mtPixyCLI"	results			output
cmp_diff "RGB to IDX"	results/rgb_99.bmp	tmp/indexed_99_rgb.bmp
cmp_diff "IDX to RGB"	results/indexed_99.bmp	tmp/rgb_99_indexed.bmp

cmp_diff "01 to 01b"	tmp/pal_01.png		tmp/pal_01b.png
cmp_diff "10 to 10b"	tmp/pal_10.png		tmp/pal_10b.png
cmp_diff "11 to 11b"	tmp/pal_11.png		tmp/pal_11b.png
cmp_diff "20 to 20b"	tmp/pal_20.png		tmp/pal_20b.png
cmp_diff "21 to 21b"	tmp/pal_21.png		tmp/pal_21b.png
cmp_diff "30 to 30b"	tmp/pal_30.png		tmp/pal_30b.png
cmp_diff "31 to 31b"	tmp/pal_31.png		tmp/pal_31b.png

