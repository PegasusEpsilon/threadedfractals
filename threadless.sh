#!/bin/bash

source config.sh

PALETTE=palette.txt
RENDERER=threadless
OUTFILE=$RENDERER
unset THREADS

source render.sh
