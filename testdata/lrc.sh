#!/bin/sh

rm ./lrc.txt

w=8
# for parityblock in {2..4}
# do
# echo $parityblock
#   for datablock in {6,8,10}
#   do
#   echo $datablock
#     for i in {1..1}
#     do
#       for((e=${parityblock};e<=${parityblock};e++))
#       do
#       packetsize=512
#       for j in {1..1}
#       do
#         ../raid/xor_example -k $datablock -p $parityblock -l 2 -s $packetsize -e $e -w $w>> lrc.txt
#         packetsize=`expr $packetsize \* 2`
#         sleep 10
#       done
#       echo >> lrc.txt
#       done
#     done
#   done
# done

for parityblock in {3..3}
do
echo $parityblock
  for datablock in {18..18}
  do
  echo $datablock
    for i in {1..1}
    do
      for((e=${parityblock};e<=${parityblock};e++))
      do
      packetsize=512
      for j in {1..1}
      do
        ../raid/xor_example -k $datablock -p $parityblock -l 2 -s $packetsize -e $e -w $w>> lrc.txt
        packetsize=`expr $packetsize \* 2`
        sleep 10
      done
      echo >> lrc.txt
      done
    done
  done
done
