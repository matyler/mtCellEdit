#!/bin/bash

. ../mtTest.txt


# Matrix, PRNG, password creation

FILE_11=$OUT/11_matrix.tsv
FILE_12=$OUT/12_prng.bin
FILE_13=$OUT/13_passwords.txt
FILE_14=$OUT/14_passwords.txt

run_sh $FILE_11 mtrdc -o $FILE_11 -create-matrix
run_sh $FILE_12 mtrdc -o $FILE_12 -iterations 10000000 -create-prng
run_sh $FILE_13 mtrdc -o $FILE_13 -pad $FILE_12 -iterations 10000 \
	-create-passwords
run_sh $FILE_14 mtrdc -o $FILE_14 -pad $FILE_12 -iterations 10000 \
	-pad-start 12345 -password-len 27 -password-chars "13579AoWrE,.*!" \
	-create-passwords


# Shuffle, XOR

FILE_21=$OUT/21_passwords.shuffle.txt
FILE_22=$OUT/22_passwords.shuffle.unshuffle.txt
FILE_23=$OUT/23_passwords.txt.xor
FILE_24=$OUT/24_passwords.txt.xor.xor
FILE_25=$OUT/25_matrix.tsv.shuffle
FILE_26=$OUT/26_matrix.tsv.shuffle.xor

run_sh $FILE_21 mtrdc -o $FILE_21 -i $FILE_13 -pad $FILE_12 -create-shuffle
run_sh $FILE_22 mtrdc -o $FILE_22 -i $FILE_21 -pad $FILE_12 -create-unshuffle
run_sh $FILE_23 mtrdc -o $FILE_23 -i $FILE_13 -pad $FILE_12 -create-xor
run_sh $FILE_24 mtrdc -o $FILE_24 -i $FILE_23 -pad $FILE_12 -create-xor
run_sh $FILE_25 mtrdc -o $FILE_25 -i $FILE_11 -pad $FILE_12 -create-shuffle
run_sh $FILE_26 mtrdc -o $FILE_26 -i $FILE_25 -pad $FILE_12 -create-xor


# Difference tests

echo
diff -q $FILE_13 $FILE_21 && txt_fail "Shuffle test 1"
diff -q $FILE_13 $FILE_22 && txt_pass "Shuffle test 2"
echo
diff -q $FILE_13 $FILE_23 && txt_fail "XOR test 1"
diff -q $FILE_13 $FILE_24 && txt_pass "XOR test 2"
echo

# Analysis of the PRNG

FILE_31=$OUT/31_analysis.txt

run_sh $FILE_31 mtrdc -i $FILE_12 -print-analysis > $FILE_31
echo
echo less $FILE_31
echo


# cedls tests
run_sh "" cedls -v $FILE_11
run_sh "" cedls -v $FILE_25
run_sh "" cedls -v $FILE_26



# On error don't exit
set +e


valg_results

