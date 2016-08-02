#!/bin/bash

source config.sh
source A0.sh

RADIUS_IMAG=$(echo "scale=40;$RADIUS_REAL*$SIZE_IMAG/$SIZE_REAL"|bc|sed -e 's/0*$//')
MSAA_REAL=$(($SIZE_REAL*$MSAA))
MSAA_IMAG=$(($SIZE_IMAG*$MSAA))
SAMPLER="julia -0.7766729 -0.13661091 crosstrap 0.125 1 0"
DIVIDER=.375
FLATTEN=


cleanup () { rm tiled.map tiled.msaa tiled.rgb; exit; }
rm tiled.map tiled.msaa tiled.rgb
mkfifo tiled.rgb tiled.msaa tiled.map
trap cleanup 1 2 4 5 6 7 8 11 13 14 15

make palette threaded modules render resample tiler pngify || exit
./palette palettes/blueglow.txt palette.bin
./tiler tiled.rgb $SIZE_REAL $SIZE_IMAG 20 20 threaded.png ./pngify/pngify {infile} {width} {height} {outfile} &
./resample tiled.msaa $SIZE_REAL $MSAA tiled.rgb &
./render $FLATTEN tiled.map palette.bin 0 $DIVIDER tiled.msaa &
echo ./threaded $THREADS $MSAA_REAL $MSAA_IMAG tiled.map $SAMPLER
time ./threaded $THREADS $MSAA_REAL $MSAA_IMAG tiled.map $SAMPLER
cleanup
