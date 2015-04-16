#!/bin/bash

source config.sh

# julia rejiggery
SAMPLER="scale 2 julia $CENTER_REAL $CENTER_IMAG crosstrap 0.125 1 0"
PALETTE=blueglow
DIVIDER=.125
SHIFT=1
OUTFILE=julia
FLATTEN=

source render.sh
