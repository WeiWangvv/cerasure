#!/bin/sh

rm ./thread.txt

w=8
datablock=10
parityblock=4
packetsize=128
echo $datablock
for parityblock in {2..4}
do
echo $parityblock
  for datablock in {4,6,8,10}
  do
for n in {1..1}
do
  echo $n
  packetsize=512
  for j in {1..1}
  do
    ../raid/xor_example -k $datablock -p $parityblock -s $packetsize -w $w -n $parityblock>> thread.txt
    # sleep 10
    packetsize=`expr $packetsize \* 2`
  done
  

done
 echo >>  thread.txt
done 
done

