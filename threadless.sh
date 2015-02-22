#!/bin/bash

source config.sh

RADIUS_IMAG=$(echo "scale=40;$RADIUS_REAL*$SIZE_IMAG/$SIZE_REAL"|bc|sed -e 's/0*$//')
MSAA_REAL=$(($SIZE_REAL*$MSAA))
MSAA_IMAG=$(($SIZE_IMAG*$MSAA))

cleanup () { rm threadless.map threadless.msaa threadless.rgb; }
cleanup
mkfifo threadless.map threadless.msaa threadless.rgb
trap "cleanup; exit" 1 2 3 4 5 6 7 8 11 13 14 15

make palette.bin threadless render resample pngify $SAMPLER || exit
./pngify threadless.rgb $SIZE_REAL $SIZE_IMAG threadless.png &
./resample threadless.msaa $SIZE_REAL $MSAA threadless.rgb &
./render $FLATTEN threadless.map palette.bin 0 $DIVIDER threadless.msaa &
echo ./threadless $MSAA_REAL $MSAA_IMAG $CENTER_REAL $CENTER_IMAG $RADIUS_REAL $RADIUS_IMAG $THETA threadless.map $SAMPLER $SAMPLER_ARGS
time ./threadless $MSAA_REAL $MSAA_IMAG $CENTER_REAL $CENTER_IMAG $RADIUS_REAL $RADIUS_IMAG $THETA threadless.map $SAMPLER $SAMPLER_ARGS
cleanup
