#!/bin/sh

rm ./alg_latency.txt

w=8
for parityblock in {4..4}
do
echo $parityblock
  for datablock in {4,6,8,10,12,14,16,18,20}
  do
  echo $datablock
    for i in {1..1}
    do
      for((e=${parityblock};e<=${parityblock};e++))
      do
      packetsize=128
      for j in {1..1}
      do
        ../raid/xor_example -k $datablock -p $parityblock -s $packetsize -e 1 -w $w>> alg_latency.txt
        packetsize=`expr $packetsize \* 2`
      done
      echo >> alg_latency.txt
      done
    done
  done
done
