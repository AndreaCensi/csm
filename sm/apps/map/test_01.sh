#!/bin/bash

head -n 100 k15l.log | ./map1 > out.log
log2pdf -config many_points.config -distance_xy 0 -in out.log
 