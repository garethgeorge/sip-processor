#!/bin/bash

HERE=`pwd`
BIN=/home/rich/bin

AZ=$1
TYPE=$2
INSTTYPE=$3

echo "running at " `/bin/date` > $HERE/$AZ-$TYPE.running

TARG=$HERE/results

GMTOFF=`date --rfc-3339="seconds" | awk -F '-' '{print $4}' | awk -F':' '{printf "%2d", $1}' | sed 's/ //'`

PERIOD=$((2592000*3))

mkdir -p $TARG

NOW=`/bin/date +%s`
NOW=$(($NOW-($GMTOFF*3600)))

# 3 month earlier
EARLIEST=$(($NOW-$PERIOD))

grep $INSTTYPE $HERE/$AZ/$TYPE.data | grep -v SUSE | sort -k 6 | awk '{print $6,$5}' | sed 's/T/ /' | sed 's/Z//' > $HERE/$AZ/$TYPE.$INSTTYPE.temp1
STEST=`wc -l $HERE/$AZ/$TYPE.$INSTTYPE.temp1 | awk '{print $1}'`
if ( test $STEST -le 0 ) ; then
	rm -f $HERE/$AZ-$TYPE.running
	exit 0
fi
$BIN/convert_time -f $HERE/$AZ/$TYPE.$INSTTYPE.temp1 | sort -n -k 1 -k 2  > $HERE/$AZ/$TYPE.$INSTTYPE.temp0
awk -v gmt=$GMTOFF '{print $1-(gmt*3600),$2}' $HERE/$AZ/$TYPE.$INSTTYPE.temp0 | awk -v early=$EARLIEST '{if($1 >= early) {print $1,$2}}' > $HERE/$AZ/$TYPE.$INSTTYPE.temp2
$BIN/bmbp_ts -f $HERE/$AZ/$TYPE.$INSTTYPE.temp2 -T -q 0.99 -c 0.01 -i 350 | grep "pred:" | awk '{print $2,$4,$6+0.0001,$14}'> $HERE/$AZ/$TYPE.$INSTTYPE.pred
awk '{print $1,$2,$3}' $HERE/$AZ/$TYPE.$INSTTYPE.pred > $HERE/$AZ/$TYPE.$INSTTYPE.temp3
$BIN/pred-distribution -f $HERE/$AZ/$TYPE.$INSTTYPE.temp3 -q 0.01 -c 0.99 -F 4.0 -I 0.05 | awk '{print $1/3600,$2}' > $TARG/$AZ-$TYPE.pgraph

	
rm -f $HERE/$AZ-$TYPE.running
