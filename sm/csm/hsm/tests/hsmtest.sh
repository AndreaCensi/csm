#!/bin/bash
set -e # exit on error


make -C ../../../

convert tunnel1.png tunnel1.pgm
convert tunnel2.png tunnel2.pgm

rm hsm0*

../../../hsm_test00 -debug 1 -in1 tunnel2.pgm -in2 tunnel1.pgm -out hsm0

for a in hsm0*pgm; do convert $a $a.png; done
	

open hsm0*png

