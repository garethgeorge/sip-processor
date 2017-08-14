#!/bin/bash

CREDPATH=~/awscred

source $CREDPATH/awsrc-west

HERE=`pwd`

$HERE/get-latest.sh 1800 > $HERE/min30-us-west-2
if ( ! test -e $HERE/onemonth-us-west-2 ) ; then
	$HERE/get-latest-month.sh > $HERE/onemonth-us-west-2
fi
cp $HERE/onemonth-us-west-2 $HERE/temp.us-west-data.txt
cat $HERE/min30-us-west-2 >> $HERE/temp.us-west-data.txt
sort $HERE/temp.us-west-data.txt > $HERE/temp1.us-west-data.txt
uniq $HERE/temp1.us-west-data.txt > $HERE/us-west-data.txt
cp $HERE/us-west-data.txt $HERE/onemonth-us-west

source $CREDPATH/awsrc-east

$HERE/get-latest.sh 1800 > $HERE/min30-us-east-2
if ( ! test -e $HERE/onemonth-us-east-2 ) ; then
	$HERE/get-latest-month.sh > $HERE/onemonth-us-east-2
fi
cp $HERE/onemonth-us-east-2 $HERE/temp.us-east-data.txt
cat $HERE/min30-us-east-2 >> $HERE/temp.us-east-data.txt
sort $HERE/temp.us-east-data.txt > $HERE/temp1.us-east-data.txt
uniq $HERE/temp1.us-east-data.txt > $HERE/us-east-data.txt
cp $HERE/us-east-data.txt $HERE/onemonth-us-east
