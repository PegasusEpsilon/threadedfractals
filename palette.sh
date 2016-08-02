#!/bin/sh

make palette pngify

. ./config.sh

./palette palettes/${1}.txt palette.bin
./pngify/pngify palette.bin 34 45 palette.png
xdg-open palette.png >/dev/null 2>&1
