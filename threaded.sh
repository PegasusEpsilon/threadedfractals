#!/bin/bash
#
# NOTE: Don't expect performance gain from this script. It runs four programs
# at the same time. threaded gets the best performance gain when it's all done
# serially. Try make threaded.png threadless.png and compare timings from
# those if you want to get real numbers. This script is more for convenience,
# and to demonstrate that you don't actually need to use intermediate files
# if you have no need of them.

source config.sh

RADIUS_IMAG=$(echo "scale=40;$RADIUS_REAL*$SIZE_IMAG/$SIZE_REAL"|bc|sed -e 's/0*$//')
MSAA_REAL=$(($SIZE_REAL*$MSAA))
MSAA_IMAG=$(($SIZE_IMAG*$MSAA))

cleanup () { rm threaded.map threaded.msaa threaded.rgb; }
cleanup
mkfifo threaded.rgb threaded.msaa threaded.map
trap "cleanup; exit" 1 2 3 4 5 6 7 8 11 13 14 15

make palette.bin threaded render resample pngify $SAMPLER || exit
./pngify threaded.rgb $SIZE_REAL $SIZE_IMAG threaded.png &
./resample threaded.msaa $SIZE_REAL $MSAA threaded.rgb &
./render $FLATTEN threaded.map palette.bin 0 $DIVIDER threaded.msaa &
echo ./threaded $THREADS $MSAA_REAL $MSAA_IMAG $CENTER_REAL $CENTER_IMAG $RADIUS_REAL $RADIUS_IMAG $THETA threaded.map $SAMPLER $SAMPLER_ARGS
time ./threaded $THREADS $MSAA_REAL $MSAA_IMAG $CENTER_REAL $CENTER_IMAG $RADIUS_REAL $RADIUS_IMAG $THETA threaded.map $SAMPLER $SAMPLER_ARGS
cleanup
