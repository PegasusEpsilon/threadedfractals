#!/bin/bash

source config.sh

# julia rejiggery
SAMPLER_ARGS="$CENTER_REAL $CENTER_IMAG $SAMPLER $SAMPLER_ARGS"
SAMPLER=julia.so
CENTER_REAL=0
CENTER_IMAG=0
DIVIDER=$(echo "scale=40;$DIVIDER/2"|bc|sed -e 's/0*$//')
echo $DIVIDER

RADIUS_IMAG=$(echo "scale=40;$RADIUS_REAL*$SIZE_IMAG/$SIZE_REAL"|bc|sed -e 's/0*$//')
MSAA_REAL=$(($SIZE_REAL*$MSAA))
MSAA_IMAG=$(($SIZE_IMAG*$MSAA))

cleanup () { rm julia.map julia.msaa julia.rgb; exit; }
rm julia.map julia.msaa julia.rgb
mkfifo julia.rgb julia.msaa julia.map
trap cleanup 1 2 3 4 5 6 7 8 11 13 14 15

make palette.bin threaded render resample pngify $SAMPLER || exit
./pngify julia.rgb $SIZE_REAL $SIZE_IMAG julia.png &
./resample julia.msaa $SIZE_REAL $MSAA julia.rgb &
./render $FLATTEN julia.map palette.bin 0 $DIVIDER julia.msaa &
time ./threaded $THREADS $MSAA_REAL $MSAA_IMAG $CENTER_REAL $CENTER_IMAG $RADIUS_REAL $RADIUS_IMAG $THETA julia.map $SAMPLER $SAMPLER_ARGS
