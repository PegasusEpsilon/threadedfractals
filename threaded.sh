#!/bin/bash
#
# NOTE: Don't expect performance gain from this script. It runs four programs
# at the same time. threaded gets the best performance gain when it's all done
# serially. Try make threaded.png threadless.png and compare timings from
# those if you want to get real numbers. This script is more for convenience,
# and to demonstrate that you don't actually need to use intermediate files
# if you have no need of them.

source config.sh

PALETTE=palette.txt
OUTFILE=threaded

source render.sh
