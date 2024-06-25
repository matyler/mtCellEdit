#!/bin/bash

. ../mtTest.txt



run_sh_cli_init



# Integer ----------------------------------------------------------------------

txt_title "mtNushUtils - Integer"
CLICOM="nushutils -integer -cli"

# ( ) ^ / * - + < <= > >= == != <=> # .
run_sh_cli 01_integer

# Variables, = += -= /= *= ^= ;
run_sh_cli 11_integer

# Functions
run_sh_cli 21_integer

# Errors
run_sh_cli 91_integer

# ------------------------------------------------------------------------------



# Float ------------------------------------------------------------------------

txt_title "mtNushUtils - Float"
CLICOM="nushutils -float -cli"

# ( ) ^ / * - + < <= > >= == != <=> # .
run_sh_cli 01_float

# Variables, = += -= /= *= ^= ;
run_sh_cli 11_float

# Functions
run_sh_cli 21_float

# Errors
run_sh_cli 91_float

# ------------------------------------------------------------------------------



# Double -----------------------------------------------------------------------

txt_title "mtNushUtils - Double"
CLICOM="nushutils -double -cli"

# ( ) ^ / * - + < <= > >= == != <=> # .
run_sh_cli 01_double

# Variables, = += -= /= *= ^= ;
run_sh_cli 11_double

# Functions
run_sh_cli 21_double

# Errors
run_sh_cli 91_double

# ------------------------------------------------------------------------------



# Rational ---------------------------------------------------------------------

txt_title "mtNushUtils - Rational"
CLICOM="nushutils -rational -cli"

# ( ) ^ / * - + < <= > >= == != <=> # .
run_sh_cli 01_rational

# Variables, = += -= /= *= ^= ;
run_sh_cli 11_rational

# Functions
run_sh_cli 21_rational

# Errors
run_sh_cli 91_rational

# ------------------------------------------------------------------------------



# Misc -------------------------------------------------------------------------

FILE_1="output/numtest.txt"
run_sh "" ./numtest						> "$FILE_1"

FILE_1="output/numbin_001"
FORMULA="2+2"
run_sh "" ./numbin "$FORMULA"		-o	"$FILE_1"a
run_sh "" ./numbin -i		"$FILE_1"a -o	"$FILE_1"b
cmp_bin "$FILE_1"ab		"$FILE_1"a	"$FILE_1"b
run_sh "" ./numbin -himp	"$FILE_1"a -o	"$FILE_1"c
cmp_bin "$FILE_1"ac		"$FILE_1"a	"$FILE_1"c
run_sh "" ./numbin -p		"$FILE_1"a			> "$FILE_1"a1
run_sh "" nushutils -integer	"$FORMULA"			> "$FILE_1"a2
cmp_diff "$FORMULA"		"$FILE_1"a1	"$FILE_1"a2

FORMULA="2^128 / 7"
FILE_2="output/numbin_002"
run_sh "" ./numbin "$FORMULA"		-o	"$FILE_2"a
run_sh "" ./numbin -i		"$FILE_2"a -o	"$FILE_2"b
cmp_bin "$FILE_2"ab		"$FILE_2"a	"$FILE_2"b
run_sh "" ./numbin -himp	"$FILE_2"a -o	"$FILE_2"c
cmp_bin "$FILE_2"ac		"$FILE_2"a	"$FILE_2"c
run_sh "" ./numbin -p		"$FILE_2"a			> "$FILE_2"a1
run_sh "" nushutils -integer	"$FORMULA"			> "$FILE_2"a2
cmp_diff "$FORMULA"		"$FILE_2"a1	"$FILE_2"a2

FILE_3="output/numbin_003"
run_sh "" ./numbin -mem		"$FILE_3"a
run_sh "" ./numbin -i		"$FILE_3"a -o	"$FILE_3"b
cmp_bin "$FILE_3"ab		"$FILE_3"a	"$FILE_3"b
run_sh "" ./numbin -himp	"$FILE_3"a -o	"$FILE_3"c
cmp_bin "$FILE_3"ac		"$FILE_3"a	"$FILE_3"c
run_sh "" ./numbin -p		"$FILE_3"a			> "$FILE_3"a1

FILE_4="output/numbin_cmp_001"
FORMULA="2^1283 / 7"
run_sh "$FILE_4"a ./numbin	"$FORMULA" -cmp	"$FILE_4"a	> "$FILE_4"a0
run_sh "" ./numbin -p		"$FILE_4"a			> "$FILE_4"a1
run_sh "" nushutils -integer	"$FORMULA"			> "$FILE_4"a2
cmp_diff "$FORMULA"		"$FILE_4"a1	"$FILE_4"a2
FORMULA="0-2^1283 / 7"
run_sh "$FILE_4"b ./numbin	"$FORMULA" -cmp	"$FILE_4"b	> "$FILE_4"b0
run_sh "" ./numbin -p		"$FILE_4"b			> "$FILE_4"b1
run_sh "" nushutils -integer	"$FORMULA"			> "$FILE_4"b2
cmp_diff "$FORMULA"		"$FILE_4"b1	"$FILE_4"b2


run_sh_fail "$FILE_4"ab cmp	"$FILE_4"a	"$FILE_4"b

# ------------------------------------------------------------------------------



# On error don't exit
set +e


valg_results


# Check results
cmp_diff "mtNushUtils" results output

