#!/bin/bash

HERE=/mnt/data
BIN=/root/bin

REGION=$1
INSTTYPE=$2
MAXSIZE=30000


if ( test -z "$REGION" ) ; then
echo "must input the region)"
exit 1
fi

if ( test -z "$INSTTYPE" ) ; then
echo "must input the specifc spot instance image type (UNIX, Windows, SUSE)"
exit 1
fi

for AZ in `cat $HERE/$REGION.az.txt` ; do
	for TYPE in `cat $HERE/instance-types.txt` ; do
		grep $INSTTYPE $HERE/$AZ/$TYPE.data | sort -k 6 | awk '{print $6,$5}' | sed 's/T/ /' | sed 's/Z//' > $HERE/$AZ/$TYPE.$INSTTYPE.temp1
		SIZE=`wc -l $AZ/$TYPE.$INSTTYPE.temp1 | awk '{print $1}'`
		if ( test $SIZE -gt $MAXSIZE ) ; then
			tail -n $MAXSIZE $HERE/$AZ/$TYPE.$INSTTYPE.temp1 > $HERE/$AZ/$TYPE.$INSTTYPE.temp2
			$BIN/convert_time -f $HERE/$AZ/$TYPE.$INSTTYPE.temp2 | sort -n -k 1 > $HERE/$AZ/$TYPE.$INSTTYPE.temp3
		else
			$BIN/convert_time -f $HERE/$AZ/$TYPE.$INSTTYPE.temp1 | sort -n -k 1 > $HERE/$AZ/$TYPE.$INSTTYPE.temp3
		fi
		$BIN/bmbp_ts -f $HERE/$AZ/$TYPE.$INSTTYPE.temp3 -T | grep pred > $HERE/$AZ/$TYPE.$INSTTYPE.pred
		awk '{print $2,$4,$6}' $HERE/$AZ/$TYPE.$INSTTYPE.pred > $HERE/$AZ/$TYPE.$INSTTYPE.temp4
		$BIN/pred-duration -T 1000000000 -f $HERE/$AZ/$TYPE.$INSTTYPE.temp4 | sort -n -k 1 | grep -v ' 0.000000'  > $HERE/$AZ/$TYPE.$INSTTYPE.temp5
		$BIN/bmbp_ts -f $HERE/$AZ/$TYPE.$INSTTYPE.temp5 -T -L -q 0.05 -c 0.95 | grep pred > $HERE/$AZ/$TYPE.$INSTTYPE.duration
	done
done

