#!/bin/bash

HERE=`pwd`

# assume that crdes are set up by aws configure
# source $CREDPATH/awsrc-west

while [ 1 ] ; do

aws ec2 describe-regions | grep 'us-' | awk '{print $3}' > $HERE/regions.txt

for REGION in `cat $HERE/regions.txt` ; do
	aws ec2 describe-availability-zones --region $REGION | awk '{print $4}' > $HERE/$REGION.az.txt
done

for REGION in `cat $HERE/regions.txt` ; do
	aws ec2 describe-availability-zones --region $REGION | awk '{print $4}' > $HERE/$REGION.az.txt

        $HERE/get-latest-vpc.sh $REGION 4500 > $HERE/min30-$REGION
      if ( ! test -e $HERE/full-vpc-$REGION ) ; then
               $HERE/get-latest-month-vpc.sh $REGION > $HERE/full-vpc-$REGION
       fi
       cp $HERE/full-vpc-$REGION $HERE/temp.$REGION
       cat $HERE/min30-$REGION >> $HERE/temp.$REGION
       sort $HERE/temp.$REGION > $HERE/temp1.$REGION
       uniq $HERE/temp1.$REGION > $HERE/$REGION-vpc-data.txt
       cp $HERE/$REGION-vpc-data.txt $HERE/full-vpc-$REGION
       ./build-predictions-vpc.sh $REGION &
done


sleep 900

done
