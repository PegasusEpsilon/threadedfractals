#!/bin/bash

source config.sh

rm threaded.map threaded.msaa threaded.rgb
make palette.bin threaded render resample pngify || exit
mkfifo threaded.map; time ./threaded 4 10 $MSAA_REAL $MSAA_IMAG $CENTER_REAL $CENTER_IMAG $RADIUS_REAL $RADIUS_IMAG $THETA threaded.map &
mkfifo threaded.msaa; ./render -l threaded.map palette.bin 0 $DIVIDER threaded.msaa &
mkfifo threaded.rgb; ./resample threaded.msaa $SIZE_REAL $MSAA threaded.rgb &
./pngify threaded.rgb $SIZE_REAL $SIZE_IMAG threaded.png
rm threaded.map threaded.msaa threaded.rgb
