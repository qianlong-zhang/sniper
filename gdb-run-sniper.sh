#./run-sniper --gdb-wait ./HPTW/hptw
#./run-sniper -c my --gdb-wait  ./HPTW/hptw
#./run-sniper -c my -n 1 --traces=./HPTW/Hptw_test/hptw.sift 2>./HPTW/Hptw_test/hptw-core.log
#./run-sniper -c gainestown -n 1 --traces=./HPTW/Mcf/mcf.sift 2>./HPTW/Mcf/mcf-core.log
#./run-sniper -c my-linked-prefetch -n 1 --traces=./link.sift 2>link.log
#./run-sniper -c my-noprefetch -n 1 --traces=./link.sift 2>link.log
./run-sniper --gdb-wait -c my-simple -n 1 --traces=./lat_mem_rd.sift 2>lat_mem_rd.log
