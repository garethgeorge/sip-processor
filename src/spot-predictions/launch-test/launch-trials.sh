#!/bin/bash

HERE=`pwd`
BIN=~rich/bin

KEY=~rich/euca-eci/rich.key
HOST=128.111.84.183

REGION=$1
TYPE=$2
DURATION=$3
INTERVAL=$4

NOW=`/bin/date`

if ( test -z "$REGION" ) ; then
	echo "must enter region type duration interval
	exit 1
fi

if ( test -z "$REGION" ) ; then
	echo "must enter region type duration interval
	exit 1
fi

if ( test -z "$DURATION" ) ; then
	echo "must enter region type duration interval
	exit 1
fi

if ( test -z "$INTERVAL" ) ; then
	echo "must enter region type duration interval
	exit 1
fi

rm -f $HERE/$REGION-$TYPE-agg.txt
scp -i $KEY root@$HOST:/mnt/data/results/$REGION-$TYPE-agg.txt $HERE

if ( ! test -e $HERE/$REGION-$TYPE-agg.txt ) ; then
	echo "$NOW couldn't transfer initial price data to $HERE/$REGION-$TYPE-agg.txt"
	exit 1
fi

rm -f $HERE/$REGION-$TYPE-pred.txt
$BIN/bmbp_ts -f $HERE/$REGION-$TYPE-agg.txt -T -i 350 -q 0.975 -c 0.01 | grep "pred:" | awk '{print $2,$4,($6+0.0001)}' > $HERE/$REGION-$TYPE-pred.txt 
PSIZE=`wc -l $HERE/$REGION-$TYPE-pred.txt | awk '{print $1}'`
if ( test $PSIZE -le 0 ) ; then
	echo "$NOW no predictions generated in $HERE/$REGION-$TYPE-pred.txt"
	exit 1
fi

rm -f $HERE/$REGION-$TYPE-pgraph.txt
$BIN/pred-distribution -f $HERE/$REGION-$TYPE-pred.txt -q 0.025 -c 0.99 -F 4.0 -I 0.05 > $HERE/$REGION-$TYPE-pgraph.txt
PSIZE=`wc -l $HERE/$REGION-$TYPE-pgraph.txt | awk '{print $1}'`
if ( test $PSIZE -le 0 ) ; then
	echo "$NOW no graph generated in $HERE/$REGION-$TYPE-pgraph.txt"
	exit 1
fi

PRICE=`awk -v dur=$DURATION '{if($1 > dur) {print $2}}' $HERE/$REGION-$TYPE-pgraph.txt | sort -n | head -n 1`


#SPOTINSTANCEREQUESTS	2015-10-14T00:23:16.000Z	Linux/UNIX	sir-021ntcxz	0.021850	open	one-time
SPOTID=`aws ec2 request-spot-instances --spot-price $PRICE --launch-specification file://./spot-skel --instance-count 1 --region $REGION | grep SPOTINSTANCEREQUESTS | awk '{print $4}'`
echo $SPOTID
sleep 5

exit 1

NOW=`date +%s`
TIMEOUT=$(($NOW+600))

while ( test $NOW -le $TIMEOUT ) ; do
	STATUS=`aws ec2 describe-spot-instance-requests --region us-east-1 | grep STATUS | awk '{print $2}'`
	if ( test "$STATUS" = "fulfilled" ) ; then
		break
	fi
	sleep 10
	NOW=`date +%s`
done


if ( ! test "$STATUS" = "fulfilled" ) ; then
	aws ec2 cancel-spot-instance-requests --spot-instance-request-ids $SPOTID --region $REGION
	euca-terminate-instances `euca-describe-instances | grep INST | awk '{print $2}'`
	echo `date` "spot instance request $SPOTID failed with status $STATUS"
	exit 1
fi




