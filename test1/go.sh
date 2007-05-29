#!/bin/bash

export PATH=$PATH:~/icra07pis/src/misc_apps/
export PATH=$PATH:~/icra07pis/src/libraries/fig/
export PATH=$PATH:../

RAYTRACER=raytracer
LD_DRAW=ld_draw
FIGMERGE=figmerge
MULTIPLY=json_pipe
NOISE=ld_noise
SM1=sm1
JSON2MATLAB=../matlab_new/json2matlab.rb

which raytracer


num=100

fig=square.fig

nrays=181
fov_deg=180
max_reading=80
NOISE_ARGS="-sigma 0.01"
RAYTRACER_ARGS="-fig $fig -nrays $nrays -fov_deg $fov_deg -max_reading $max_reading"

pose1="5 5 0"
pose2="5.1 5.2 0.09"

echo $RAYTRACER $RAYTRACER_ARGS

echo $pose1 | $RAYTRACER $RAYTRACER_ARGS > scan1.txt
echo $pose2 | $RAYTRACER $RAYTRACER_ARGS > scan2.txt


$MULTIPLY -n $num < scan1.txt > scan1-noise.txt
$MULTIPLY -n $num < scan2.txt | $NOISE > scan2-noise.txt

$LD_DRAW < scan1.txt > scan1.fig
$LD_DRAW -config scan2.config < scan2.txt > scan2.fig

$FIGMERGE $fig scan1.fig > scan1-map.fig
$FIGMERGE $fig scan2.fig > scan2-map.fig
$FIGMERGE $fig scan1.fig scan2.fig > complete.fig

$SM1 -file1 scan1-noise.txt -file2 scan2-noise.txt -config sm1.config > results.txt

$JSON2MATLAB results < results.txt > results.m
