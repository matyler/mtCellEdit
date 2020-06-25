#!/bin/bash

. ../mtTest.txt


txt_title "mtDWUtils"


CP="cp -af"
DB="db"

rm -rf $DB/*

echo "butt add buckets 5" | mtdwcli -q -db $DB


# Soda
SODA_DIR_A="soda_a"
SODA_DIR_TMP="soda_tmp"
SODA_DIR_B="soda_b"

$CP ../../COPYING.txt $SODA_DIR_A/in_01.txt
$CP ../../COPYING.txt $SODA_DIR_A/in_02.txt
$CP ../../COPYING.txt $SODA_DIR_A/in_03.txt

run_sh "Soda Encode" dwencsoda -db $DB -o $SODA_DIR_TMP	$SODA_DIR_A/in_*.txt
run_sh "Soda Decode" dwdecsoda -db $DB -o $SODA_DIR_B	$SODA_DIR_TMP/in_*.soda


# Bottle
BOTTLE_DIR_A="bottle_a"
BOTTLE_DIR_TMP="bottle_tmp"
BOTTLE_DIR_B="bottle_b"

$CP ../../COPYING.txt $BOTTLE_DIR_A/in_01.txt
$CP ../../COPYING.txt $BOTTLE_DIR_A/in_02.txt
$CP ../../COPYING.txt $BOTTLE_DIR_A/in_03.txt

BOTTLE="$BOTTLE_DIR_TMP/bottle.png"

pixynew -width 1000 -height 1000 -imtype rgb -otype png "" -o $BOTTLE

run_sh "Bottle Encode" dwencbot -db $DB -bottle $BOTTLE \
	-o $BOTTLE_DIR_TMP	$BOTTLE_DIR_A/in_*.txt

run_sh "Bottle Decode" dwdecbot -db $DB -bottle $BOTTLE \
	-o $BOTTLE_DIR_B	$BOTTLE_DIR_TMP/in_*.png


# Homoglyphs
HG_DIR_A="hg_a"
HG_DIR_TMP="hg_tmp"
HG_DIR_B="hg_b"

pixynew -width 1900 -height 1900 -imtype rgb -otype png "" -o $HG_DIR_A/in_a.png
pixynew -width 2100 -height 2100 -imtype rgb -otype png "" -o $HG_DIR_A/in_b.png
pixynew -width 2300 -height 2300 -imtype rgb -otype png "" -o $HG_DIR_A/in_c.png

BOTTLE="../../COPYING.txt"

run_sh "Homoglyph Encode" dwenchg -db $DB -bottle $BOTTLE \
	-o $HG_DIR_TMP	$HG_DIR_A/in_*.png

run_sh "Homoglyph Decode" dwdechg -db $DB \
	-o $HG_DIR_B	$HG_DIR_TMP/in_*.txt


# Fonts
FONT_DIR_A="font_a"
FONT_DIR_TMP="font_tmp"
FONT_DIR_B="font_b"

BOTTLE="../../COPYING.txt"


for (( NUM=1; NUM<=13; NUM++ ))
do
	FILE=$(printf "in_%02i.txt" $NUM)

	$CP $BOTTLE $FONT_DIR_A/$FILE

	run_sh "Font Encode $NUM" dwencfont -db $DB -font $NUM \
		-o $FONT_DIR_TMP	$FONT_DIR_A/$FILE
done


run_sh "Font Decode" dwdecfont -db $DB -o $FONT_DIR_B $FONT_DIR_TMP/in_*.txt



# On error don't exit
set +e


valg_results


# Check results
cmp_diff "mtDWUtils Soda"	$SODA_DIR_A	$SODA_DIR_B
cmp_diff "mtDWUtils Bottle"	$BOTTLE_DIR_A	$BOTTLE_DIR_B
cmp_diff "mtDWUtils Homoglyphs"	$HG_DIR_A	$HG_DIR_B
cmp_diff "mtDWUtils Fonts"	$FONT_DIR_A	$FONT_DIR_B

