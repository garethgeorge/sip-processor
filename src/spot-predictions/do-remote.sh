#!/bin/bash

KEY=/home/rich/euca-eci/rich.key
HOST=128.111.84.197

HERE=`pwd`

CREDPATH=~/awscred

source $CREDPATH/awsrc-west

while [ 1 ] ; do

aws ec2 describe-regions | grep 'us-' | awk '{print $3}' > regions.txt

for REGION in `cat regions.txt` ; do
	aws ec2 describe-availability-zones --region $REGION | awk '{print $4}' > $REGION.az.txt
done

for REGION in `cat regions.txt` ; do
	aws ec2 describe-availability-zones --region $REGION | awk '{print $4}' > $HERE/$REGION.az.txt

        $HERE/get-latest.sh $REGION 4500 > $HERE/min30-$REGION
      if ( ! test -e $HERE/full-$REGION ) ; then
               $HERE/get-latest-month.sh $REGION > $HERE/full-$REGION
       fi
       cp $HERE/full-$REGION $HERE/temp.$REGION
       cat $HERE/min30-$REGION >> $HERE/temp.$REGION
       sort $HERE/temp.$REGION > $HERE/temp1.$REGION
       uniq $HERE/temp1.$REGION > $HERE/$REGION-data.txt
       cp $HERE/$REGION-data.txt $HERE/full-$REGION

done

scp -i $KEY *-data.txt root@$HOST:/mnt/data
scp -i $KEY instance-types.txt root@$HOST:/mnt/data
scp -i $KEY regions.txt root@$HOST:/mnt/data
scp -i $KEY index.html root@$HOST:/mnt/data
scp -i $KEY table.css root@$HOST:/mnt/data
scp -i $KEY *.az.txt root@$HOST:/mnt/data

for REGION in `cat regions.txt | grep us-west` ; do
	ssh -i $KEY root@$HOST "cd /mnt/data; ./build-html.sh $REGION" &
done

for REGION in `cat regions.txt | grep us-east` ; do
	ssh -i $KEY root@$HOST "cd /mnt/data; ./build-html.sh $REGION" &
done

sleep 900

done
