#!/bin/bash

source config.sh

rm threadless.map threadless.msaa threadless.rgb
make palette.bin threadless render resample pngify || exit
mkfifo threadless.map; time ./threadless 4 10 $MSAA_REAL $MSAA_IMAG $CENTER_REAL $CENTER_IMAG $RADIUS_REAL $RADIUS_IMAG $THETA threadless.map &
mkfifo threadless.msaa; ./render -l threadless.map palette.bin 0 $DIVIDER threadless.msaa &
mkfifo threadless.rgb; ./resample threadless.msaa $SIZE_REAL $MSAA threadless.rgb &
./pngify threadless.rgb $SIZE_REAL $SIZE_IMAG threadless.png
rm threadless.map threadless.msaa threadless.rgb
