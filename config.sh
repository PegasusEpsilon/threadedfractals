#!/bin/bash

THREADS=$(cat /proc/cpuinfo | grep processor | wc -l)
MSAA=1
SIZE_REAL=1920
SIZE_IMAG=1080
CENTER_REAL=0
CENTER_IMAG=0
SAMPLER="translate -0.75 0 mandelbrot renormalized escape_count 16"
DIVIDER=10
FLATTEN=-l
PALETTE=blueglow.txt
RENDERER=threaded
