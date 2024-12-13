#!/bin/bash

path="../../data_release/check_*_*.root"

for file in $path
do
    temp=${file#*_}
    runnum=${temp%%_*}
    
    ./ana $runnum &
done