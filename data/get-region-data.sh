#! /bin/bash

HERE=/mnt/data

REGION=$1

echo REGION: $REGION

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
