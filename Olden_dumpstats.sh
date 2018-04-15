#!/bin/bash
CURRENT_PATH=`pwd`

for bench in `ls ./output_test/`
do
    echo
    echo
    OUT_PATH=$CURRENT_PATH/mnt_sdc/output_health/${bench}
    echo $OUT_PATH
    echo "Getting $bench"
    cd $OUT_PATH
    for dir in `ls ./`
    do
        cd $OUT_PATH/${dir}
        pwd
        for dir1 in `ls ./`
        do
            cd $OUT_PATH/${dir}/${dir1}
            pwd
            echo "../../../../../tools/dumpstats.py >all.out"
            ../../../../../tools/dumpstats.py >all.out
        done
    done
done
