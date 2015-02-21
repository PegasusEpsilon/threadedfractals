#!/bin/bash

THREADS=$(cat /proc/cpuinfo | grep processor | wc -l)
MSAA=16
SIZE_REAL=$((1*1920))
SIZE_IMAG=$((1*1080))
CENTER_REAL=-0.7766729
CENTER_IMAG=-0.13661091
RADIUS_REAL=0.00016
THETA=0
DIVIDER=1
SAMPLER=renormalized.so
