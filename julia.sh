#!/bin/bash

source config.sh

# julia rejiggery
SAMPLER="julia -0.7766729 -0.13661091 crosstrap 0.125 1 0"
DIVIDER=.375
OUTFILE=julia
FLATTEN=

source render.sh
