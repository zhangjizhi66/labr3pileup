#!/bin/bash

#path="../../CSNS202304/event_*_*.root"
#path="../../CSNS202310/event_*_*.root"
path="../../CSNS202311/event_*_*.root"

for file in $path
do
    temp=${file#*_}
    runnum=${temp%%_*}
    
    ./ana $runnum &
done