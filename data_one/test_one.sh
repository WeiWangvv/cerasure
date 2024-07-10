for w in {4,8}
do
    for parityblock in {2..4}
    do
        for datablock in {4..10}
        do
            e=1
            packsize=262144
            # echo $datablock 
            # echo $parityblock
            # echo $w
            ../raid/xor_example -k $datablock -p $parityblock -s $packsize -e $e -w $w
            sleep 10
        done
    done
done