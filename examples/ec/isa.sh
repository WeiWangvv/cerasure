#!/bin/sh
#删除已有文件


rm isa_l.txt
#开始保存运行结果
for parityblock in {2..4}
do
  echo $parityblock
  for datablock in {4,6,8,10}
  do
    echo $datablock
    ./ec_simple_example  -k $datablock -p $parityblock -e 1 >> isa_l.txt
    echo >> isa_l.txt
    sleep 10
  done
done


