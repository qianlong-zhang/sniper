#!/bin/bash
export SNIPER_ROOT=`pwd`
./run-sniper -c ./config/numa/numa.cfg -d numa-out --viz -n 8 -- Mytest/link_test

