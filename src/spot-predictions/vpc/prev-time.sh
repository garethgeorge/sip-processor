#!/bin/bash

if ( test -z "$1" ) ; then
	echo "must supply number of seconds into the past"
	exit 1
fi

# 2014-12-31T21:17:31.000Z

NOW=`date "+%Y-%m-%d %H:%M:%S"`

rm -f zzznow
echo $NOW > zzznow
TIME=`convert_time -f zzznow`
NEWTIME=`echo $TIME | awk -v offset=$1 '{print $1-offset}'`
#echo $NEWTIME
PREVTIME=`ptime $NEWTIME`


date --date="$PREVTIME" "+%Y-%m-%dT%H:%M:%S.000Z"
