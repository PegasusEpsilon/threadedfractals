#!/bin/bash

THREADS=$(cat /proc/cpuinfo | grep processor | wc -l)
MSAA=16
SIZE_REAL=$((1*1920))
SIZE_IMAG=$((1*1080))
CENTER_REAL=-0.75
CENTER_IMAG=0
RADIUS_REAL=2
THETA=0
SAMPLER=crosstrap.so
SAMPLER_ARGS="0.5 1 0"
DIVIDER=1.5
FLATTEN=
