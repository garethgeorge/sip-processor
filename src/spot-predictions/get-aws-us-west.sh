#!/bin/bash

CREDPATH=~/awscred

source $CREDPATH/awsrc-west
HERE=`pwd`

while [ 1 ] ; do

for REGION in `aws ec2 describe-regions | grep us-west | awk '{print $3}'` ; do
	#
	# save off the AZs to prevent another AWS call by other scripts
	#
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
	$HERE/ship-html.sh $REGION &
done

sleep 4700

done


