#!/bin/sh




rm ./wide.txt

w=8
for parityblock in {3,4}
do
echo $parityblock
  for datablock in {24,48,72,96}
  do
  echo $datablock
    for n in {2,4,6,8,12,16}
    do
      packetsize=128
      for j in {1..11}
      do
        ../raid/xor_example -k $datablock -p $parityblock -s $packetsize -e $parityblock -w $w -n $n>> wide.txt
        packetsize=`expr $packetsize \* 2`
        sleep 10
      done
      echo >> wide.txt
    done
    echo >> wide.txt
  done
done