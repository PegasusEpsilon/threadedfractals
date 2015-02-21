#!/bin/bash

source config.sh

RADIUS_IMAG=$(echo "scale=40;$RADIUS_REAL*$SIZE_IMAG/$SIZE_REAL"|bc|sed -e 's/0*$//')
MSAA_REAL=$(($SIZE_REAL*$MSAA))
MSAA_IMAG=$(($SIZE_IMAG*$MSAA))

cleanup () { rm threaded.map threaded.msaa threaded.rgb; exit; }
mkfifo threaded.rgb threaded.msaa threaded.map
trap cleanup 1 2 3 4 5 6 7 8 11 13 14 15

make palette.bin threaded render resample pngify || exit
./pngify threaded.rgb $SIZE_REAL $SIZE_IMAG threaded.png &
./resample threaded.msaa $SIZE_REAL $MSAA threaded.rgb &
./render -l threaded.map palette.bin 0 $DIVIDER threaded.msaa &
time ./threaded $SAMPLER $THREADS $MSAA_REAL $MSAA_IMAG $CENTER_REAL $CENTER_IMAG $RADIUS_REAL $RADIUS_IMAG $THETA threaded.map
