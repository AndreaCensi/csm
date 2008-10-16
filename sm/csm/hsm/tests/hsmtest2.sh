#!/bin/bash
set -e # exit on error


make -C ../../../

convert tunnel1.png tunnel1.pgm
convert tunnel2.png tunnel2.pgm

rm -f hsm2*

echo Self test
../../../hsm_test00 -debug 1 -in1 tunnel2.pgm  -out hsm2  -hsm_num_angular_hypotheses 2

for a in hsm2*pgm; do convert $a $a.png; done
	

open hsm2*png

