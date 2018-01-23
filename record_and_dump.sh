#echo "recording link test ...  log is in link.log"
#./record-trace -o link -- HPTW/Link_test/link_test &>link.log
#echo "dumping link test ...  , log is in link.dump"
#./sift/siftdump link.sift &>link.dump

#echo "recording mcf test ...  log is in mcf.log"
#./record-trace -o mcf -- HPTW/Mcf/mcf HPTW/Mcf/inp.in &>mcf.log
#echo "dumping mcf test ...  , log is in mcf.dump"
#./sift/siftdump mcf.sift &>mcf.dump

echo "recording miss test ...  log is in miss.log"
./record-trace -o miss -- HPTW/Miss/miss  &>miss.log
echo "dumping miss test ...  , log is in miss.dump"
./sift/siftdump miss.sift &>miss.dump
