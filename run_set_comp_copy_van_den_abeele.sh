#!/bin/bash

./waf --run "lorawan-long-term-test --nNodes=100 --dr=$2" &> $3

python fix_output_format_comp.py $3 $2

./waf --run "lorawan-long-term-test --nNodes=500 --dr=$2" &> $3

python fix_output_format_comp.py $3 $2

./waf --run "lorawan-long-term-test --nNodes=1000 --dr=$2" &> $3

python fix_output_format_comp.py $3 $2

./waf --run "lorawan-long-term-test --nNodes=5000 --dr=$2" &> $3

python fix_output_format_comp.py $3 $2

./waf --run "lorawan-long-term-test --nNodes=10000 --dr=$2" &> $3

python fix_output_format_comp.py $3 $2

mv out$2* $1
