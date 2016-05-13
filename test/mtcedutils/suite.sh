#!/bin/bash

. ../mtTest.txt


OUT=output
CHECK=results



# Creative

FILE_11=$OUT/11_set.tsv
FILE_12=$OUT/12_append.tsv
FILE_13=$OUT/13_paste.tsv

run_sh $FILE_11 cedset	"r1c1 qwert" "r1c2 Qwert" "r1c3 589.23" \
		"r2c1 13-4-2011 13:59:01" "r2c2 -1.2" "r2c3 $%^" \
		"r3c4 =sum( r1c1:r[-1]c[-1] )" \
		-o $FILE_11

run_sh $FILE_12 cedappend $FILE_11 $FILE_11 -col $FILE_11 -o $FILE_12

run_sh $FILE_13 cedpaste $FILE_11 \
		-dest r8c8:r20c15 $FILE_11 \
		-dest r25c2 $FILE_11 \
		-dest r30c3:r35c8 -range r2c2:r_c_ $FILE_11 \
		-o $FILE_13


# Destructive

FILE_21=$OUT/21_clear.tsv
FILE_22=$OUT/22_cut.tsv
FILE_23=$OUT/23_insert.tsv

run_sh $FILE_21 cedclear -range r10c10:r17c13 $FILE_13 -o $FILE_21

run_sh $FILE_22 cedcut -col -start 5 -total 2 $FILE_21 -o $FILE_22

run_sh $FILE_23 cedinsert -start 13 -total 2 $FILE_22 -o $FILE_23


# Transformative

FILE_31=$OUT/31_flip.tsv
FILE_32=$OUT/32_sort.tsv
FILE_33=$OUT/33_transpose.tsv
FILE_34=$OUT/34_rotate.tsv

run_sh $FILE_31 cedflip $FILE_23 -o $FILE_31

run_sh $FILE_32 cedsort -i $FILE_31 -col -start 6 -total 7 "16,ac" -o $FILE_32

run_sh $FILE_33 cedtranspose $FILE_32 -o $FILE_33

run_sh $FILE_34 cedrotate $FILE_33 -o $FILE_34


# Reflective

FILE_41=$OUT/41_eval.tsv
FILE_42=$OUT/42_find.tsv
FILE_43=$OUT/43_ls.tsv

echo $FILE_41
run_sh "" cedeval -v -i $FILE_34 "sum( r1c1:r_c_ )" > $FILE_41

echo $FILE_42
run_sh "" cedfind -v -i $FILE_34 "qwert" > $FILE_42

echo $FILE_43
run_sh "" cedls -v $OUT/[1-3]*.tsv > $FILE_43


# Misc
FILE_51=$OUT/51_ensemble.tsv
FILE_52=$OUT/52_csv.csv
FILE_53=$OUT/53_tsv_output.tsv
FILE_54=$OUT/54_tsv_output_noq.tsv
FILE_55=$OUT/55_diff_01.txt
FILE_56=$OUT/56_diff_02.txt
FILE_57=$OUT/57_diff_03.txt

run_sh $FILE_51 cedutils -i $FILE_34 -com set "r9c3 Hello World" "r9c4 -951.357" \
	-com insert -start 4 "" -o $FILE_51

run_sh $FILE_52 cedutils -i $FILE_51 -otype csv_content -o $FILE_52

run_sh $FILE_53 cedutils -i $FILE_51 -otype output_tsv_q -o $FILE_53

run_sh $FILE_54 cedutils -i $FILE_51 -otype output_tsv -o $FILE_54

echo $FILE_55
run_sh "" ceddiff $FILE_11 $FILE_12 > $FILE_55

echo $FILE_56
run_sh "" ceddiff $FILE_12 $FILE_11 > $FILE_56

echo $FILE_57
run_sh "" ceddiff -csv $FILE_52 -tsv $FILE_53 > $FILE_57


# On error don't exit
set +e


valg_results


# Check results
cmp_diff "mtCedUtils" $CHECK $OUT

