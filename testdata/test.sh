#!/bin/sh

w=8
for datablock in {4..10}
do
  for parityblock in {2..2}
  do
    e=0
    packsize=128
    for i in {1..8} 
    do
      rm xorMatch/xorMatch_${datablock}_${parityblock}_${packsize}_${w}_${e}_2.txt
      packsize=`expr $packsize \* 2`
    done
  done
done

for datablock in {4..10}
do
echo $datablock 
  for parityblock in {2..2}
  do
    for i in {1..1}
    do
      e=0
      packsize=128
      for j in {1..8}
      do
        ../raid/xor_example -k $datablock -p $parityblock -s $packsize -e $e -w $w>> xorMatch/xorMatch_${datablock}_${parityblock}_${packsize}_${w}_${e}_2.txt
        packsize=`expr $packsize \* 2`
        sleep 10
      done
    done
  done
done


