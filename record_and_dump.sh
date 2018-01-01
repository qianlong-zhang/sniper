echo "recording link test ...  log is in link.log"
./record-trace -o link -- HPTW/Link_test/link_test &>link.log
echo "dumping link test ...  , log is in link.dump"
./sift/siftdump link.sift &>link.dump
