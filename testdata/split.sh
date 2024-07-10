#!/bin/sh



for n in {4,8,12,16}
do
    w=8
    datablock=96
    parityblock=4
    e=0
    packsize=128
    for i in {1..11}
    do
        rm t_split/new_split_decode_${n}_${datablock}_${parityblock}_${packsize}_${w}_${e}.txt
        packsize=`expr $packsize \* 2`
    done
    echo '--------------------------'
    echo $w
    echo '--------------------------'

    atablock=96
    echo $datablock
    parityblock=4
    echo $parityblock
    e=0
    packsize=128
    for j in {1..11}
    do
        ../raid/xor_example -k $datablock -p $parityblock -s $packsize -w $w -n $n>> t_split/new_split_decode_${n}_${datablock}_${parityblock}_${packsize}_${w}_${e}.txt
        packsize=`expr $packsize \* 2`
        sleep 10
    done
done