#!/bin/bash

source A0.sh

cleanup () { rm tiled.map tiled.msaa tiled.rgb; exit; }

mkfifo tiled.rgb tiled.msaa tiled.map

trap cleanup 1 2 4 5 6 7 8 11 13 14 15

make palette.bin threaded render resample tiler pngify || exit
./tiler tiled.rgb $SIZE_REAL $SIZE_IMAG 20 20 threaded.png ./pngify {infile} {width} {height} {outfile} &
./resample tiled.msaa $SIZE_REAL $MSAA tiled.rgb &
./render -l tiled.map palette.bin 0 $DIVIDER tiled.msaa &
time ./threaded $THREADS $MSAA_REAL $MSAA_IMAG $CENTER_REAL $CENTER_IMAG $RADIUS_REAL $RADIUS_IMAG $THETA tiled.map
