#!/bin/bash

THREADS=$(cat /proc/cpuinfo | grep processor | wc -l)
MSAA=1
SIZE_REAL=1920
SIZE_IMAG=1080

# full set
CENTER_REAL=-0.75
CENTER_IMAG=0
RADIUS_REAL=2.2

# nautilus
CENTER_REAL=-0.7766729
CENTER_IMAG=-0.13661091
RADIUS_REAL=0.00016

# "bird of paradise" stress test
#CENTER_REAL=0.3750001200618655
#CENTER_IMAG=0.2166393884377127
#RADIUS_REAL=0.000000000002

SAMPLER="scale $RADIUS_REAL translate $CENTER_REAL $CENTER_IMAG mandelbrot renormalized escape_count 16"
DIVIDER=.3
FLATTEN=-l

SAMPLER="scale 1.8 rotate -18 renormalized julia $CENTER_REAL $CENTER_IMAG escape_count 16"
DIVIDER=3
FLATTEN=-2

SAMPLER="scale 2 translate -.76 .59 rotate 11.25 dragon"
FLATTEN=-l

PALETTE=palette
RENDERER=threaded


