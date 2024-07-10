#!/bin/sh

rm ./cerasure.txt

w=8
for parityblock in {4..4}
do
echo $parityblock
  for datablock in {10..10}
  do
  echo $datablock
    for i in {1..1}
    do
      for((e=${parityblock};e<=${parityblock};e++))
      do
      packetsize=128
      for j in {1..11}
      do
        ../raid/xor_example -k $datablock -p $parityblock -s $packetsize -e 1 -w $w>> cerasure.txt
        packetsize=`expr $packetsize \* 2`
        # sleep 10
      done
      echo >> cerasure.txt
      done
    done
  done
done
