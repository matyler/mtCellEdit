#!/bin/bash

. ../mtTest.txt



# mtString

FILE_11=$OUT/11_mtstring.txt
run_sh "" ./mtstring > $FILE_11

FILE_12=$OUT/11_mtstring_res1.txt
cat $FILE_11 | wc > $FILE_12
RES_12=

FILE_13=$OUT/11_mtstring_res2.txt
cat $FILE_11 | sort -u | wc > $FILE_13


cmp_arg "mtString 1" "2" $(cat "$FILE_12" | awk '{print $1}')
cmp_arg "mtString 2" "1" $(cat "$FILE_13" | awk '{print $1}')



# On error don't exit
set +e

valg_results



echo

