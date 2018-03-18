#!/bin/bash
CURRENT_PATH=`pwd`
BENCH_PATH=$CURRENT_PATH/Test_prog/Olden/Olden-compiled
#PAGE_SIZE=2M
PAGE_SIZE=1G
DATE_NAME=$(date +%Y%m%d%H%M)
#DATE_NAME=$(date +%Y%m%d)
CONFIG_PATH=my/
#CONFIG_0=my-linked-only-count

CONFIG_1=my-tlbfree-1
CONFIG_2=my-tlbfree-2
CONFIG_3=my-tlbfree-3
CONFIG_4=my-tlbfree-4

CONFIG_9=my-linkednew-1
CONFIG_10=my-linkednew-2
CONFIG_11=my-linkednew-3
CONFIG_12=my-linkednew-4
#CONFIG_17=my-ghb
CONFIG_18=my-no
#CONFIG_19=my-simple

#CONFIG_20=my-tlbfree-perfect-1
#CONFIG_21=my-tlbfree-perfect-2
#CONFIG_22=my-tlbfree-perfect-3
#CONFIG_23=my-tlbfree-perfect-4
#CONFIG_13=my-linked-1
#CONFIG_14=my-linked-2
#CONFIG_15=my-linked-3
#CONFIG_16=my-linked-4
#CONFIG_5=my-tlbfreerw-1
#CONFIG_6=my-tlbfreerw-2
#CONFIG_7=my-tlbfreerw-3
#CONFIG_8=my-tlbfreerw-4

echo $OUTPUT


cd $BENCH_PATH
#for bench in `ls -d */`
for bench in "health/"
do
    echo "Creating dir for output_test"
    echo $bench
    for config in $CONFIG_0 $CONFIG_1 $CONFIG_2 $CONFIG_3 $CONFIG_4 $CONFIG_5 $CONFIG_6 $CONFIG_7 $CONFIG_8 $CONFIG_9 $CONFIG_10 $CONFIG_11 $CONFIG_12 $CONFIG_13  $CONFIG_14 $CONFIG_15 $CONFIG_16 $CONFIG_17 $CONFIG_18 $CONFIG_19 $CONFIG_20 $CONFIG_21 $CONFIG_22 $CONFIG_23
    do
        echo
        echo
        OUT_PATH=$CURRENT_PATH/mnt_sdc/output_health/${bench}${DATE_NAME}_${PAGE_SIZE}/$config
        #echo $OUT_PATH
        if [ -d $OUT_PATH ];
        then
            echo "$OUT_PATH already exist"
        else
            echo "mkdir -p $OUT_PATH"
            mkdir -p $OUT_PATH 
        fi
        #echo "Running"
        cd $CURRENT_PATH
        case $bench in
            "bh/")
                echo "running bh"
                echo "./run-sniper -c  $CONFIG_PATH$config -d  $OUT_PATH    --  $BENCH_PATH/${bench}bh 4000 &> $OUT_PATH/log"                  >$OUT_PATH/log
                setsid ./run-sniper -c  $CONFIG_PATH$config -d  $OUT_PATH    --  $BENCH_PATH/${bench}bh 4000 &> $OUT_PATH/log                 &
                ;;
            "bisort/")
                echo "running bisort"
                echo "./run-sniper -c  $CONFIG_PATH$config -d  $OUT_PATH    --  $BENCH_PATH/${bench}bisort 250000 &> $OUT_PATH/log"            >$OUT_PATH/log
                setsid ./run-sniper -c  $CONFIG_PATH$config -d  $OUT_PATH    --  $BENCH_PATH/${bench}bisort 250000 &> $OUT_PATH/log           &
                ;;
            "em3d/")
                echo "running em3d"
                echo "./run-sniper -c  $CONFIG_PATH$config -d  $OUT_PATH    --  $BENCH_PATH/${bench}em3d  20000 10 125 &> $OUT_PATH/log"       >$OUT_PATH/log
                setsid ./run-sniper -c  $CONFIG_PATH$config -d  $OUT_PATH    --  $BENCH_PATH/${bench}em3d  20000 10 125 &> $OUT_PATH/log      &
                ;;
            "health/")
                echo "running health"
                echo "./run-sniper -c  $CONFIG_PATH$config -d  $OUT_PATH    --  $BENCH_PATH/${bench}health 5 500 4 &> $OUT_PATH/log"           >$OUT_PATH/log
                setsid ./run-sniper -c  $CONFIG_PATH$config -d  $OUT_PATH    --  $BENCH_PATH/${bench}health 5 500 4 &> $OUT_PATH/log          &
                ;;
            "mst/")
                echo "running mst"
                echo "./run-sniper -c  $CONFIG_PATH$config -d  $OUT_PATH    --  $BENCH_PATH/${bench}mst  1024 &> $OUT_PATH/log"                >$OUT_PATH/log
                setsid ./run-sniper -c  $CONFIG_PATH$config -d  $OUT_PATH    --  $BENCH_PATH/${bench}mst  1024 &> $OUT_PATH/log               &
                ;;
            "perimeter/")
                echo "running perimeter"
                echo "./run-sniper -c  $CONFIG_PATH$config -d  $OUT_PATH    --  $BENCH_PATH/${bench}perimeter &> $OUT_PATH/log"                >$OUT_PATH/log
                setsid ./run-sniper -c  $CONFIG_PATH$config -d  $OUT_PATH    --  $BENCH_PATH/${bench}perimeter &> $OUT_PATH/log               &
                ;;
            "power/")
                echo "running power"
                echo "./run-sniper -c  $CONFIG_PATH$config -d  $OUT_PATH    --  $BENCH_PATH/${bench}power &> $OUT_PATH/log"                    >$OUT_PATH/log
                setsid ./run-sniper -c  $CONFIG_PATH$config -d  $OUT_PATH    --  $BENCH_PATH/${bench}power &> $OUT_PATH/log                   &
                ;;
            "treeadd/")
                echo "running treeadd"
                echo "./run-sniper -c  $CONFIG_PATH$config -d  $OUT_PATH    --  $BENCH_PATH/${bench}treeadd 18 &> $OUT_PATH/log"               >$OUT_PATH/log
                setsid ./run-sniper -c  $CONFIG_PATH$config -d  $OUT_PATH    --  $BENCH_PATH/${bench}treeadd 18 &> $OUT_PATH/log              &
                ;;
            "tsp/")
                echo "running tsp"
                echo "./run-sniper -c  $CONFIG_PATH$config -d  $OUT_PATH    --  $BENCH_PATH/${bench}tsp 100000 &> $OUT_PATH/log"               >$OUT_PATH/log
                setsid ./run-sniper -c  $CONFIG_PATH$config -d  $OUT_PATH    --  $BENCH_PATH/${bench}tsp 100000 &> $OUT_PATH/log              &
                ;;
            "voronoi/")
                echo "running voronoi"
                echo "./run-sniper -c  $CONFIG_PATH$config -d  $OUT_PATH    --  $BENCH_PATH/${bench}voronoi 600 20 32 7  &> $OUT_PATH/log"     >$OUT_PATH/log
                setsid ./run-sniper -c  $CONFIG_PATH$config -d  $OUT_PATH    --  $BENCH_PATH/${bench}voronoi 600 20 32 7  &> $OUT_PATH/log    &
                ;;
        esac
    done

done
