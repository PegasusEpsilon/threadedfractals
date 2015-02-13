#!/bin/bash

source A0.sh

make palette.bin threaded render resample tiler pngify || exit
rm threaded.map threaded.msaa threaded.rgb
mkfifo threaded.rgb
./tiler threaded.rgb $SIZE_REAL $SIZE_IMAG 20 20 threaded.png ./pngify {infile} {width} {height} {outfile} &
mkfifo threaded.msaa
./resample threaded.msaa $SIZE_REAL $MSAA threaded.rgb &
mkfifo threaded.map
./render -l threaded.map palette.bin 0 $DIVIDER threaded.msaa &
time ./threaded $THREADS $MSAA_REAL $MSAA_IMAG $CENTER_REAL $CENTER_IMAG $RADIUS_REAL $RADIUS_IMAG $THETA threaded.map
rm threaded.map threaded.msaa threaded.rgb
