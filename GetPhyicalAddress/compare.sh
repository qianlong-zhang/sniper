#!/bin/bash
# $1 pid
# $2 time interval

count=0

while [ 1 ];
do
    sudo ./a.out --combine $1 > temp_$count
    count=$(($count+1))
    sleep $2                                                                                                                                                                                    
done
