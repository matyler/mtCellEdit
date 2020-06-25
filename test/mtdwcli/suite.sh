#!/bin/bash

. ../mtTest.txt


txt_title "mtDWCLI"
CLICOM="mtdwcli -db d0 -q -t"

run_sh_cli_init


# Preparations - setup empty directories, etc
. ./suite_00.txt

# Scriptlets via run_sh_cli - General: well, soda, tap
. ./suite_11.txt

# Scriptlets via run_sh_cli - Homoglyphs & fonts
. ./suite_12.txt

# Scriptlets via run_sh_cli - /tmp/
. ./suite_21.txt


valg_results

# Check results - log files
cmp_diff "mtDWCLI Log" results output

