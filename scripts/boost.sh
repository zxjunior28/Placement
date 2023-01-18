rm -rf build/
cmake -S . -B build/ -DBUILD_EXAMPLES=ON
cmake --build build/ -j4
./build/bin/Lab3 $@ $@
# "data/case0.txt"
./verifier "data/case1.txt" "output.txt" 
python3 draw.py "data/case1.txt" "output.txt" "data/fig/case1"
