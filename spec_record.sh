#!/bin/bash
CURRENTPATH=`pwd`

#out_sift/SPEC_2017/500.perbench_r/run_base_refrate_mytest-m64.0000
#500.perbench_r="../../../../record-trace -o perlbench_r_base.mytest-m64 -- ./perlbench_r_base.mytest-m64 -I./lib checkspam.pl 2500 5 25 11 150 1 1 1 1"
500CMD="test"
echo $500CMD

SPECLIST="500.perbench_r  502.gcc_r  505.mcf_r  520.omnetpp_r  523.xalancbmk_r  525.x264_r  531.deepsjeng_r  541.leela_r  548.exchange2_r  557.xz_r"
OUTPATH=./out_sift/SPEC_2017/
EXEPATH=out_sift/SPEC_2017/500.perbench_r/run_base_refrate_mytest-m64.0000

for app in  $SPECLIST
do
    echo $app
    cd ${CURRENTPATH}/out_sift/SPEC_2017/${app}/run_base_refrate_mytest-m64.0000
    pwd
    echo `eval echo '$'"$app"`
done


500: setsid ../../../../record-trace -o perlbench_r_base.mytest-m64 -- ./perlbench_r_base.mytest-m64 -I./lib checkspam.pl 2500 5 25 11 150 1 1 1 1 > checkspam.2500.5.25.11.150.1.1.1.1.out 2>> checkspam.2500.5.25.11.150.1.1.1.1.err
502: setsid  ../../../../record-trace -o cpugcc_r_base.mytest-m64 -- ./cpugcc_r_base.mytest-m64 gcc-pp.c -O3 -finline-limit=0 -fif-conversion -fif-conversion2 -o gcc-pp.opts-O3_-finline-limit_0_-fif-conversion_-fif-conversion2.s
505: setsid ../../../../record-trace -o mcf_r_base.mytest-m64 ./mcf_r_base.mytest-m64 inp.in  > inp.out 2>> inp.err
520: setsid ../../../../record-trace -o omnetpp_r_base.mytest-m64  ./omnetpp_r_base.mytest-m64 -c General -r 0 > omnetpp.General-0.out 2>> omnetpp.General-0.err
523: setsid ../../../../record-trace -o cpuxalan_r_base.mytest-m64 ./cpuxalan_r_base.mytest-m64 -v t5.xml xalanc.xsl > ref-t5.out 2>> ref-t5.err
525: setsid ../../../../record-trace -o x264_r_base.mytest-m64 ./x264_r_base.mytest-m64 --pass 1 --stats x264_stats.log --bitrate 1000 --frames 1000 -o BuckBunny_New.264 BuckBunny.yuv 1280x720 > run_000-1000_x264_r_base.mytest-m64_x264_pass1.out 2>> run_000-1000_x264_r_base.mytest-m64_x264_pass1.err
531: setsid ../../../../record-trace -o deepsjeng_r_base.mytest-m64 ./deepsjeng_r_base.mytest-m64 ref.txt > ref.out 2>> ref.err
541: setsid ../../../../record-trace -o leela_r_base.mytest-m64 ./leela_r_base.mytest-m64 ref.sgf > ref.out 2>> ref.err
548: setsid ../../../../record-trace -o exchange2_r_base.mytest-m64 ./exchange2_r_base.mytest-m64 6 > exchange2.txt 2>> exchange2.err
557: setsid ../../../../record-trace -o xz_r_base.mytest-m64 ./xz_r_base.mytest-m64 cld.tar.xz 160 19cf30ae51eddcbefda78dd06014b4b96281456e078ca7c13e1c0c9e6aaea8dff3efb4ad6b0456697718cede6bd5454852652806a657bb56e07d61128434b474 59796407 61004416 6 > cld.tar-160-6.out 2>> cld.tar-160-6.err
