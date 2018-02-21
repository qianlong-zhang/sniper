#!/bin/bash
CURRENT_PATH=`pwd`
BENCH_PATH=$CURRENT_PATH/Test_prog/Olden/Olden-compiled
DATE_NAME=$(date +%Y%m%d%H%M)
#DATE_NAME=$(date +%Y%m%d)
CONFIG_PATH=my/
CONFIG_0=my-linked-only-count
CONFIG_1=my-tlbfree-1
CONFIG_2=my-tlbfree-2
CONFIG_3=my-tlbfree-3
CONFIG_4=my-tlbfree-4
CONFIG_5=my-tlbfreerw-1
CONFIG_6=my-tlbfreerw-2
CONFIG_7=my-tlbfreerw-3
CONFIG_8=my-tlbfreerw-4
CONFIG_9=my-linkednew-1
CONFIG_10=my-linkednew-2
CONFIG_11=my-linkednew-3
CONFIG_12=my-linkednew-4
CONFIG_13=my-ghb
CONFIG_14=my-no
echo $OUTPUT
HEALTH_CMD="health 5 500 4"

#setsid ./run-sniper -c my-tlbfree-1 -d output/health/20180219/tlbfree-1/ -- Test_prog/Olden/Olden-compiled/health/health 5 500 4 &>output/health/20180219/tlbfree-1/log
#setsid ./run-sniper -c my-tlbfree-2 -d output/health/20180219/tlbfree-2/ -- Test_prog/Olden/Olden-compiled/health/health 5 500 4 &>output/health/20180219/tlbfree-2/log
#setsid ./run-sniper -c my-tlbfree-3 -d output/health/20180219/tlbfree-3/ -- Test_prog/Olden/Olden-compiled/health/health 5 500 4 &>output/health/20180219/tlbfree-3/log
#setsid ./run-sniper -c my-tlbfree-4 -d output/health/20180219/tlbfree-4/ -- Test_prog/Olden/Olden-compiled/health/health 5 500 4 &>output/health/20180219/tlbfree-4/log

cd $BENCH_PATH
#for bench in `ls -d */`
for bench in "health/"
do
    echo "Creating dir for output_test"
    echo $bench
    for config in $CONFIG_0 $CONFIG_1 $CONFIG_2 $CONFIG_3 $CONFIG_4 $CONFIG_5 $CONFIG_6 $CONFIG_7 $CONFIG_8 $CONFIG_9 $CONFIG_10 $CONFIG_11 $CONFIG_12 $CONFIG_13  $CONFIG_14
    do
        OUT_PATH=$CURRENT_PATH/output_test/$bench$DATE_NAME/$config
        echo $OUT_PATH
        if [ -d $OUT_PATH ];
        then
            echo "$OUT_PATH already exist"
        else
            echo "mkdir -p $OUT_PATH"
            mkdir -p $OUT_PATH 
        fi
    done

    echo "Running"
    cd $CURRENT_PATH
    case $bench in
        "bh/")
            ;;
        "bisort/")
            ;;
        "em3d/")
            ;;
        "health/")
            echo "running health"
#setsid ./run-sniper -c my-tlbfree-1 -d output/health/20180219/tlbfree-1/ -- Test_prog/Olden/Olden-compiled/health/health 5 500 4 &>output/health/20180219/tlbfree-1/log
#setsid ./run-sniper -c my-tlbfree-2 -d output/health/20180219/tlbfree-2/ -- Test_prog/Olden/Olden-compiled/health/health 5 500 4 &>output/health/20180219/tlbfree-2/log
#setsid ./run-sniper -c my-tlbfree-3 -d output/health/20180219/tlbfree-3/ -- Test_prog/Olden/Olden-compiled/health/health 5 500 4 &>output/health/20180219/tlbfree-3/log
#setsid ./run-sniper -c my-tlbfree-4 -d output/health/20180219/tlbfree-4/ -- Test_prog/Olden/Olden-compiled/health/health 5 500 4 &>output/health/20180219/tlbfree-4/log
            ;;
        "mst/")
            ;;
        "perimeter/")
            ;;
        "power/")
            ;;
        "treeadd/")
            ;;
        "tsp/")
            ;;
        "voronoi/")
            ;;
    esac
done
