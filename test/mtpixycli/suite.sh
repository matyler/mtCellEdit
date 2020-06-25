#!/bin/bash

. ../mtTest.txt


txt_title "mtPixyCLI"


CLICOM="mtpixycli -seed 123 -q -t -patterns /usr/share/mtpixy-qt5/patterns/default.png -shapes /usr/share/mtpixy-qt5/shapes/default.png"
run_sh_cli_init


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

