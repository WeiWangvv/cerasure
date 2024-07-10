

e=0
w=8
datablock=4
parityblock=2
packsize=128
for i in {1..11}
do
rm point_time/finally_${datablock}_${parityblock}_${packsize}_${w}_${e}.txt
packsize=`expr $packsize \* 2`
done
echo '--------------------------'
echo '--------------------------'


echo $datablock
echo $parityblock
packsize=128
for j in {1..11}
do
    ../raid/xor_example -k $datablock -p $parityblock -s $packsize -e $e -w $w>> point_time/finally_${datablock}_${parityblock}_${packsize}_${w}_${e}.txt
    packsize=`expr $packsize \* 2`
    sleep 10
done

datablock=6
parityblock=3
packsize=128
for i in {1..11}
do
rm point_time/finally_${datablock}_${parityblock}_${packsize}_${w}_${e}.txt
packsize=`expr $packsize \* 2`
done
echo '--------------------------'
echo '--------------------------'


echo $datablock
echo $parityblock
packsize=128
for j in {1..11}
do
    ../raid/xor_example -k $datablock -p $parityblock -s $packsize -e $e -w $w>> point_time/finally_${datablock}_${parityblock}_${packsize}_${w}_${e}.txt
    packsize=`expr $packsize \* 2`
    sleep 10
done

datablock=10
parityblock=4
packsize=128
for i in {1..11}
do
rm point_time/finally_${datablock}_${parityblock}_${packsize}_${w}_${e}.txt
packsize=`expr $packsize \* 2`
done
echo '--------------------------'
echo '--------------------------'


echo $datablock
echo $parityblock
packsize=128
for j in {1..11}
do
    ../raid/xor_example -k $datablock -p $parityblock -s $packsize -e $e -w $w>> point_time/finally_${datablock}_${parityblock}_${packsize}_${w}_${e}.txt
    packsize=`expr $packsize \* 2`
    sleep 10
done