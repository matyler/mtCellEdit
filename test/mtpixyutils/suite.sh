#!/bin/bash

. ../mtTest.txt


OUT=output
CHECK=results



# Prepare input files
cd input
./get_part.sh
cd ..



# Creative - BMP
FILE_11=$OUT/11_indexed.bmp
FILE_12=$OUT/12_rgb.bmp
run_sh $FILE_11 pixynew -imtype indexed -otype bmp "" -o $FILE_11
run_sh $FILE_12 pixynew -imtype rgb -otype bmp "" -o $FILE_12

# Creative - PNG
FILE_13=$OUT/13_indexed.png
FILE_14=$OUT/14_rgb.png
run_sh $FILE_13 pixynew -imtype indexed -otype png "" -o $FILE_13
run_sh $FILE_14 pixynew -imtype rgb -otype png "" -o $FILE_14

# Creative - GIF
FILE_15=$OUT/15_indexed.gif
run_sh $FILE_15 pixynew -imtype indexed -otype gif "" -o $FILE_15

# Creative - JPEG
FILE_16=$OUT/16_rgb.jpeg
run_sh $FILE_16 pixynew -imtype rgb -otype jpeg "" -o $FILE_16



# Loader - BMP
FILE_21=$OUT/21_indexed.bmp
FILE_22=$OUT/22_rgb.bmp
run_sh $FILE_21 pixyutils -i $FILE_11 -otype bmp -o $FILE_21
run_sh $FILE_22 pixyutils -i $FILE_12 -otype bmp -o $FILE_22

# Loader - PNG
FILE_23=$OUT/23_indexed.png
FILE_24=$OUT/24_rgb.png
run_sh $FILE_23 pixyutils -i $FILE_13 -otype png -o $FILE_23
run_sh $FILE_24 pixyutils -i $FILE_14 -otype png -o $FILE_24

# Loader - GIF
FILE_25=$OUT/25_indexed.gif
run_sh $FILE_25 pixyutils -i $FILE_15 -otype gif -o $FILE_25

# Loader - JPEG
FILE_26=$OUT/26_rgb.jpeg
run_sh $FILE_26 pixyutils -i $FILE_16 -otype jpeg -o $FILE_26



cmp_diff "$FILE_11" $FILE_11 $FILE_21
cmp_diff "$FILE_12" $FILE_12 $FILE_22
cmp_diff "$FILE_13" $FILE_13 $FILE_23
cmp_diff "$FILE_14" $FILE_14 $FILE_24
cmp_diff "$FILE_15" $FILE_15 $FILE_25
cmp_diff "$FILE_16" $FILE_16 $FILE_26



# Palettes - Uniform 2-6, grey, size 2-256
FILE_31=$OUT/31_pal_01.gpl
FILE_32=$OUT/32_pal_02.gpl
FILE_33=$OUT/33_pal_03.gpl
FILE_34=$OUT/34_pal_04.gpl
FILE_35=$OUT/35_pal_05.gpl
FILE_36=$OUT/36_pal_06.gpl
run_sh $FILE_31 pixynew -palette 1 -otype gpl "" -o $FILE_31
run_sh $FILE_32 pixynew -palette 2 -otype gpl "" -o $FILE_32
run_sh $FILE_33 pixynew -palette 3 -otype gpl "" -o $FILE_33
run_sh $FILE_34 pixynew -palette 4 -otype gpl "" -o $FILE_34
run_sh $FILE_35 pixynew -palette 5 -otype gpl "" -o $FILE_35
run_sh $FILE_36 pixynew -palette 6 -otype gpl "" -o $FILE_36



# Limits - valid side (width, height)
FILE_41=$OUT/41_limit.png
FILE_42=$OUT/42_limit.png
FILE_43=$OUT/43_limit.png
run_sh $FILE_41 pixynew -width 1 -height 1 -otype png "" -o $FILE_41
run_sh $FILE_42 pixynew -width 1 -height 32767 -otype png "" -o $FILE_42
run_sh $FILE_43 pixynew -width 32767 -height 1 -otype png "" -o $FILE_43



# Process manually prepared files for checking later
cd input
FILES=$(ls _*)
cd ..

echo "$FILES" |
while read line
do
	run_sh "$line" pixyutils -i input/$line -otype none -o output/$line
done



# On error don't exit
set +e

echo



# Limits - invalid side (width, height, palette)
run_sh_fail "Error - width   = -1"	pixynew -width -1	""
run_sh_fail "Error - width   = 0"	pixynew -width 0	""
run_sh_fail "Error - width   = 32768"	pixynew -width 32768	""
run_sh_fail "Error - width   = 32769"	pixynew -width 32769	""

run_sh_fail "Error - height  = -1"	pixynew -height -1	""
run_sh_fail "Error - height  = 0"	pixynew -height 0	""
run_sh_fail "Error - height  = 32768"	pixynew -height 32768	""
run_sh_fail "Error - height  = 32769"	pixynew -height 32769	""

run_sh_fail "Error - palette = -1"	pixynew -palette -1	""
run_sh_fail "Error - palette = 7"	pixynew -palette 7	""



# Load broken files
ls input/empty input/part_* |
while read line
do
	run_sh_fail "$line" pixyutils -i $line
done



valg_results


# Check results
cmp_diff "mtPixyUtils" $CHECK $OUT

