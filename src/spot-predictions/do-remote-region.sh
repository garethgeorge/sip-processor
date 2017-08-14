#!/bin/bash

KEY=/home/rich/euca-eci/rich.key
HOST=128.111.84.197

HERE=`pwd`

while [ 1 ] ; do

rm -f regions2.txt
grep "us-" us-*price*.txt | awk '{print $2}' | sort | uniq | sed 's/reg-//' > regions2.txt
awk '{print $3}' us-*price*.txt | sort | uniq > instances.txt

scp -i $KEY regions2.txt root@$HOST:/mnt/data
scp -i $KEY instances.txt root@$HOST:/mnt/data
scp -i $KEY us*-price*.txt root@$HOST:/mnt/data
#scp -i $KEY index.html root@$HOST:/mnt/data
scp -i $KEY table.css root@$HOST:/mnt/data
scp -i $KEY build-region-html.sh root@$HOST:/mnt/data

for REGION in `cat regions2.txt` ; do
	ssh -i $KEY root@$HOST "cd /mnt/data; ./build-region-html.sh $REGION" &
done

sleep 300

done
