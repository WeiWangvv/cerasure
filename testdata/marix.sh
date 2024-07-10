#!/bin/sh


e=0
w=8
for datablock in {4..10}
do
for parityblock in {2..4}
do
    packsize=128
    for i in {1..11}
    do
    rm t_matrix/greedy_encode_${datablock}_${parityblock}_${packsize}_${w}_${e}.txt
    packsize=`expr $packsize \* 2`
    done
done
done
echo '--------------------------'
echo $w
echo '--------------------------'

for datablock in {4..10}
do
echo $datablock
for parityblock in {2..4}
do
echo $parityblock
    for i in {1..1}
    do
    packsize=128
    for j in {1..11}
    do
        ../raid/xor_example -k $datablock -p $parityblock -s $packsize -e $e -w $w>> t_matrix/greedy_encode_${datablock}_${parityblock}_${packsize}_${w}_${e}.txt
        packsize=`expr $packsize \* 2`
        sleep 10
    done
    done
done
done

