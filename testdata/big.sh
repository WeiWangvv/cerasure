#!/bin/sh

w=8
for datablock in {8,16,32,48,64,80,96}
do
  for parityblock in {2..4}
  do
    e=0
    packsize=128
    for i in {1..11}
    do
        rm data_big/big_${datablock}_${parityblock}_${packsize}_${w}_${e}.txt
        packsize=`expr $packsize \* 2`
    done
  done
done

for datablock in {8,16,32,48,64,80,96}
do
  for parityblock in {2..4}
  do
    for i in {1..1}
    do
      e=0
      packsize=128
      for i in {1..11}
      do
      packsize=`expr $packsize \* 2`
      ../raid/xor_example -k $datablock -p $parityblock -s $packsize -e $e -w $w>> data_big/big_${datablock}_${parityblock}_${packsize}_${w}_${e}.txt
      sleep 10
    done
    done
  done
done


