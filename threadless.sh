#!/bin/bash

source config.sh

PALETTE=palette
RENDERER=threadless
OUTFILE=$RENDERER
unset THREADS

source render.sh
