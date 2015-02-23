#!/bin/bash
#
# NOTE: Don't expect performance gain from this script. It runs four programs
# at the same time. ${OUTFILE} gets the best performance gain when it's all done
# serially. Try make ${OUTFILE}.png threadless.png and compare timings from
# those if you want to get real numbers. This script is more for convenience,
# and to demonstrate that you don't actually need to use intermediate files
# if you have no need of them.

MSAA_REAL=$(($SIZE_REAL*$MSAA))
MSAA_IMAG=$(($SIZE_IMAG*$MSAA))

cleanup () { rm ${OUTFILE}.map ${OUTFILE}.msaa ${OUTFILE}.rgb; }
cleanup
mkfifo ${OUTFILE}.rgb ${OUTFILE}.msaa ${OUTFILE}.map
trap "cleanup; exit" 1 2 3 4 5 6 7 8 11 13 14 15

make palette $RENDERER modules render resample pngify || exit
./palette $PALETTE palette.bin
./pngify ${OUTFILE}.rgb $SIZE_REAL $SIZE_IMAG ${OUTFILE}.png &
./resample ${OUTFILE}.msaa $SIZE_REAL $MSAA ${OUTFILE}.rgb &
./render $FLATTEN ${OUTFILE}.map palette.bin 0 $DIVIDER ${OUTFILE}.msaa &
echo ./$RENDERER $THREADS $MSAA_REAL $MSAA_IMAG ${OUTFILE}.map $SAMPLER $SAMPLER_ARGS
time ./$RENDERER $THREADS $MSAA_REAL $MSAA_IMAG ${OUTFILE}.map $SAMPLER $SAMPLER_ARGS
cleanup
