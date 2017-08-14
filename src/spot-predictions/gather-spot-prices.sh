#!/bin/bash

BIN=/home/rich/bin
HERE=`pwd`

while [ 1 ] ; do
	$HERE/get-spot-prices.sh > zzzprice
	grep "us-east" zzzprice >> us-east-price.txt
	grep "us-west " zzzprice >> us-west-price.txt
	grep "us-west-2 " zzzprice >> us-west-2-price.txt
	SLEEPTIME=`$BIN/normal -V -m 311 -s 60 -c 1 -S | awk '{printf "%4.0f",$1}'`
	if ( test $SLEEPTIME -lt 0 ) ; then
		SLEEPTIME=302
	fi
	sleep $SLEEPTIME
done
	

