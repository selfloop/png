rm output.txt

./run_tests.sh cmake-build-debug/bin/verifypn-linux64 "-n" output.txt

sort output.txt > sorted.txt

cmp sorted.txt pg-output.txt

rm sorted.txt