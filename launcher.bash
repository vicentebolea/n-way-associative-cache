#!/bin/bash

cache_capacity=(1 2 4 8 16 32)
associativity=(1 2 4 8 16)
block_size=(8 16 32 64 128)

# Empty the output files
> output1.data
> output2.data
> output3.data

#Compute for cache capacity vs cache misses
for i in ${cache_capacity[@]}; do
  ./cache_sim $i 1 32 ls.trace | grep 'misses' | cut -d ' ' -f 4 | xargs printf "%d %s\n" $i >> output1.data 
done

#Compute for associativity vs cache misses
for i in ${associativity[@]}; do
  ./cache_sim 8 $i 32 ls.trace | grep 'misses' | cut -d ' ' -f 4 | xargs printf "%d %s\n" $i >> output2.data 
done

#Compute for block size vs cache misses
for i in ${block_size[@]}; do
  ./cache_sim 8 1 $i ls.trace | grep 'misses' | cut -d ' ' -f 4 | xargs printf "%d %s\n" $i >> output3.data 
done
