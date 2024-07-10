#!/bin/sh

packsize=128
for j in {1..11}
do
    ../raid/xor_example -s $packsize >> xor_test/xor__${packsize}.txt
    packsize=`expr $packsize \* 2`
    sleep 10
done