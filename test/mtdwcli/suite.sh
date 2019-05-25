#!/bin/bash

. ../mtTest.txt


txt_title "mtDWCLI"


CLICOM="mtdwcli -db d0 -q -t"
run_sh_cli_init


# Start afresh
rm -rf {d0,d1,d2,d3,r1,r2,r3}/*


# d0: mkdata, create butts from data in this repo
run_sh_cli 01

# d1: create original data files in d1
./mkdata d0

# r1: create random files & mtdwcli scripts
#./rndfile -db d0 -path r1 -quiet -tot 1000 -min 1000 -max 100000
#./rnd_r2.sh > tmp/r2.txt
#./rnd_r3.sh > tmp/r3.txt


cp -a ../../COPYING.txt d1/in.001.txt
cp -a ../../COPYING.txt d1/in.002.txt
cp -a ../../COPYING.txt d1/in.003.txt
cp -a ../../COPYING.txt d1/in.004.txt
cp -a ../../COPYING.txt d1/in.005.txt
cp -a ../../COPYING.txt d1/in.006.txt

> d1/000.bin
run_sh_cli 11


# d2 = d1 -> encode/encrypted
run_sh_cli 21


# d3 = d2 -> decoded/decrypted
run_sh_cli 31


# r1 -> r2
#run_sh_cli ../tmp/r2

# r2 -> r3
#run_sh_cli ../tmp/r3


# Check results
#cmp_diff "mtDWCLI d1->d3"	d1	d3
#cmp_diff "mtDWCLI r1->r3"	r1	r3


echo
diff -q d1/000.bin	d2/000.bin.soda			&& txt_fail "000.bin"
diff -q d1/001.bin	d2/001.bin.soda			&& txt_fail "001.bin"
diff -q d1/002.bin	d2/002.bin.soda			&& txt_fail "002.bin"
diff -q d1/003.bin	d2/003.bin.soda			&& txt_fail "003.bin"
diff -q d1/004.bin	d2/004.bin.soda			&& txt_fail "004.bin"
diff -q d1/in.001.txt	d2/in.001.txt.soda		&& txt_fail "in.001.txt"
diff -q d1/005.bin	d2/005.bin.soda			&& txt_fail "005.bin"
diff -q d1/006.bin	d2/006.bin.soda			&& txt_fail "006.bin"
diff -q d1/007.bin	d2/007.bin.soda			&& txt_fail "007.bin"
diff -q d1/010.bin	d2/010.bin.soda			&& txt_fail "010.bin"
diff -q d1/011.bin	d2/011.bin.soda			&& txt_fail "011.bin"
diff -q d0/bottle.flac	d2/bottle.006.soda.flac		&& txt_fail "bottle.flac"
diff -q d0/bottle.png	d2/bottle.007.soda.png		&& txt_fail "bottle.png"
diff -q d1/in.002.txt	d2/in.002.txt.soda		&& txt_fail "in.002.txt"
diff -q d1/in.003.txt	d2/in.003.txt.soda		&& txt_fail "in.003.txt"
diff -q d1/in.004.txt	d2/in.004.txt.soda		&& txt_fail "in.004.txt"
diff -q d1/in.005.txt	d2/in.005.txt.soda		&& txt_fail "in.005.txt"
diff -q d1/in.006.txt	d2/in.006.txt.soda		&& txt_fail "in.006.txt"
diff -q d0/bottle.flac	d2/bottle.in.005.soda.flac	&& txt_fail "bottle.flac"
diff -q d0/bottle.png	d2/bottle.in.006.soda.png	&& txt_fail "bottle.png"
echo

valg_results

# Check results
cmp_diff "mtDWCLI Log" results output

