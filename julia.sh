#!/bin/bash

source config.sh

# julia rejiggery
SAMPLER="scale $RADIUS_REAL julia $CENTER_REAL $CENTER_IMAG crosstrap 0.125 1 0"
PALETTE=blueglow
DIVIDER=.375
SHIFT=0
OUTFILE=julia
FLATTEN=

source render.sh
