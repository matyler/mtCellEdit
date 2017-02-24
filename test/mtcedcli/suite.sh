#!/bin/bash

. ../mtTest.txt


OUT=output
CHECK=results
SHEXE=scripts
CEDCOM="mtcedcli -q -t"


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


# Destructive
run_sh_cli 21


# Transformative
run_sh_cli 31


# Reflective
run_sh_cli 41


# Misc
run_sh_cli 51


# Hand checked output
LOGFILE=tmp/91.log.txt
> $LOGFILE
run_sh_cli 91


# Soak tests
SOAK_INPUT=tmp/soak
./mtgentex mtgentex.txt > $SOAK_INPUT.txt
run_sh_cli ../$SOAK_INPUT

# Difference tests

echo
diff -q tmp/soak_01.tsv tmp/soak_02.tsv && txt_fail "Soak undo 0"
diff -q tmp/soak_01.tsv tmp/soak_03.tsv && txt_pass "Soak undo 1"
diff -q tmp/soak_01.tsv tmp/soak_05.tsv && txt_pass "Soak undo 2"
diff -q tmp/soak_02.tsv tmp/soak_04.tsv && txt_pass "Soak undo 3"
echo



valg_results


# Check results
cmp_diff "mtCedCLI" $CHECK $OUT

