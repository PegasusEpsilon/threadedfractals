#!/bin/bash

source config.sh

# dragon rejiggery
SAMPLER="scale 2 translate -.76 .59 rotate 11.25 dragon"
DIVIDER=3
FLATTEN=-l
OUTFILE=dragon

source render.sh
