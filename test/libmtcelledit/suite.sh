#!/bin/bash

. ../mtTest.txt


txt_title "libmtCellEdit"


# tcread

run_sh "" ./tcreadst
run_sh "" ./tcreadmt


# tcwrite

run_sh "" ./tcwritest
run_sh "" ./tcwritemt



# On error don't exit
set +e

valg_results



echo

