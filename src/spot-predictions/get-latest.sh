#!/bin/bash

REGION=$1
INTERVAL=$2	

if ( test -z "$1" ) ; then
	echo "must enter inverval in seconds"
fi

if ( test -z "$REGION" ) ; then
	echo "must enter region"
	exit 1
fi

if ( test -z "$INTERVAL" ) ; then
	echo "must enter interval"
	exit 1
fi

NOW=`date "+%Y-%m-%dT%H:%M:%S"`

PREVTIME=`./prev-time.sh $INTERVAL`


#echo $NOW $ONEYEAR
#aws ec2 describe-spot-price-history --region $REGION --instance-types `cat instance-types.txt` --start-time $PREVTIME --end-time $NOW
aws ec2 describe-spot-price-history --region $REGION --start-time $PREVTIME --end-time $NOW --product-description="Linux/UNIX" | grep Linux | grep -v SUSE
#aws ec2 describe-spot-price-history --region $REGION --start-time $PREVTIME --end-time $NOW | grep Linux | grep -v SUSE

