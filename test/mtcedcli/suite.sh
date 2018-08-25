#!/bin/bash

. ../mtTest.txt


txt_title "mtCedCLI"


OUT=output
CHECK=results

CLICOM="mtcedcli -q -t"
run_sh_cli_init


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
mtgentex mtgentex.txt > $SOAK_INPUT.txt
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

