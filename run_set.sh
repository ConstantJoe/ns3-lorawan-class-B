#!/bin/bash

./waf --run "lorawan-long-term-test --nNodes=1 --dr=$2" &> $3

python fix_output_format.py $3

./waf --run "lorawan-long-term-test --nNodes=2 --dr=$2" &> $3

python fix_output_format.py $3

./waf --run "lorawan-long-term-test --nNodes=4 --dr=$2" &> $3

python fix_output_format.py $3

./waf --run "lorawan-long-term-test --nNodes=8 --dr=$2" &> $3

python fix_output_format.py $3

./waf --run "lorawan-long-term-test --nNodes=16 --dr=$2" &> $3

python fix_output_format.py $3

./waf --run "lorawan-long-term-test --nNodes=32 --dr=$2" &> $3

python fix_output_format.py $3

./waf --run "lorawan-long-term-test --nNodes=64 --dr=$2" &> $3

python fix_output_format.py $3

./waf --run "lorawan-long-term-test --nNodes=128 --dr=$2" &> $3

python fix_output_format.py $3

./waf --run "lorawan-long-term-test --nNodes=256 --dr=$2" &> $3

python fix_output_format.py $3

./waf --run "lorawan-long-term-test --nNodes=512 --dr=$2" &> $3

python fix_output_format.py $3

# having trouble with very large simulations
./waf --run "lorawan-long-term-test --nNodes=1024 --dr=$2" &> $3

python fix_output_format.py $3


mv out* $1
