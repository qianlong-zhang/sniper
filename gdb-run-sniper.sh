#./run-sniper --gdb-wait ./HPTW/hptw
#./run-sniper -c mydunnington --gdb-wait  ./HPTW/hptw
#./run-sniper -c mydunnington -n 1 --traces=./HPTW/Hptw_test/hptw.sift 2>./HPTW/Hptw_test/hptw-core.log
#./run-sniper -c gainestown -n 1 --traces=./HPTW/Mcf/mcf.sift 2>./HPTW/Mcf/mcf-core.log
#./run-sniper -c mydunnington-linked-prefetch -n 1 --traces=./link.sift 2>link.log
#./run-sniper -c mydunnington-noprefetch -n 1 --traces=./link.sift 2>link.log
./run-sniper -c mydunnington-simple-prefetch -n 1 --traces=./link.sift 2>link.log
