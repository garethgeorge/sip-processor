#!/bin/bash

REGION=$1

if ( test -z "$REGION" ) ; then
	echo "must enter region"
	exit 1
fi


NOW=`date "+%Y-%m-%dT%H:%M:%S"`
PREVMONTH=`date "+%m" | awk '{print $1-1}'`
PREVYEAR=`date "+%Y" | awk '{print $1-1}'`
ONEYEAR="$PREVYEAR-`date "+%m-%dT%H:%M:%S"`"
ONEMONTH="`date "+%Y"`-$PREVMONTH-`date "+%dT%H:%M:%S"`"


#echo $NOW $ONEYEAR
#aws ec2 describe-spot-price-history --region $REGION --instance-types `cat instance-types.txt` --start-time $ONEYEAR --end-time $NOW
aws ec2 describe-spot-price-history --region $REGION --start-time $ONEYEAR --end-time $NOW --product-description "Linux/UNIX (Amazon VPC)"
#aws ec2 describe-spot-price-history --region $REGION --start-time $ONEYEAR --end-time $NOW 

