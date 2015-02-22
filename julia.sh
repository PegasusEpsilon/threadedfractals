#!/bin/bash

THREADS=$(cat /proc/cpuinfo | grep processor | wc -l)
MSAA=1
SIZE_REAL=$((1*1920))
SIZE_IMAG=$((1*1080))
CENTER_REAL=0
CENTER_IMAG=0
RADIUS_REAL=2
THETA=0
SAMPLER=julia.so
SAMPLER_ARGS="-0.7766729 -0.13661091 crosstrap.so 0.5 1 0"
DIVIDER=1.5
FLATTEN=

RADIUS_IMAG=$(echo "scale=40;$RADIUS_REAL*$SIZE_IMAG/$SIZE_REAL"|bc|sed -e 's/0*$//')
MSAA_REAL=$(($SIZE_REAL*$MSAA))
MSAA_IMAG=$(($SIZE_IMAG*$MSAA))

cleanup () { rm threaded.map threaded.msaa threaded.rgb; exit; }
rm threaded.map threaded.msaa threaded.rgb
mkfifo threaded.rgb threaded.msaa threaded.map
trap cleanup 1 2 3 4 5 6 7 8 11 13 14 15

make palette.bin threaded render resample pngify $SAMPLER || exit
./pngify threaded.rgb $SIZE_REAL $SIZE_IMAG threaded.png &
./resample threaded.msaa $SIZE_REAL $MSAA threaded.rgb &
./render $FLATTEN threaded.map palette.bin 0 $DIVIDER threaded.msaa &
time ./threaded $THREADS $MSAA_REAL $MSAA_IMAG $CENTER_REAL $CENTER_IMAG $RADIUS_REAL $RADIUS_IMAG $THETA threaded.map $SAMPLER $SAMPLER_ARGS
