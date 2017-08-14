#!/bin/bash

HERE=/mnt/data
BIN=/root/bin

REGION=$1
INSTTYPE=$2
#MAXSIZE=30000
#MAXSIZE=15000
MAXSIZE=5000


if ( test -z "$REGION" ) ; then
echo "must input the region)"
exit 1
fi

if ( test -z "$INSTTYPE" ) ; then
echo "must input the specifc spot instance image type (UNIX, Windows, SUSE)"
exit 1
fi

PERIOD=$((2592000*3))

for AZ in `cat $HERE/$REGION.az.txt` ; do
	for TYPE in `cat $HERE/instance-types.txt` ; do
		grep $INSTTYPE $HERE/$AZ/$TYPE.data | sort -k 6 | awk '{print $6,$5}' | sed 's/T/ /' | sed 's/Z//' > $HERE/$AZ/$TYPE.$INSTTYPE.temp1
#		SIZE=`wc -l $AZ/$TYPE.$INSTTYPE.temp1 | awk '{print $1}'`
#		if ( test $SIZE -gt $MAXSIZE ) ; then
#			tail -n $MAXSIZE $HERE/$AZ/$TYPE.$INSTTYPE.temp1 > $HERE/$AZ/$TYPE.$INSTTYPE.temp2
#			$BIN/convert_time -f $HERE/$AZ/$TYPE.$INSTTYPE.temp2 | sort -n -k 1 > $HERE/$AZ/$TYPE.$INSTTYPE.temp3
#		else
#			$BIN/convert_time -f $HERE/$AZ/$TYPE.$INSTTYPE.temp1 | sort -n -k 1 > $HERE/$AZ/$TYPE.$INSTTYPE.temp3
#		fi
		$BIN/convert_time -f $HERE/$AZ/$TYPE.$INSTTYPE.temp1 | sort -n -k 1 > $HERE/$AZ/$TYPE.$INSTTYPE.temp3
		END=`tail -n 1 $HERE/$AZ/$TYPE.$INSTTYPE.temp3 | awk '{print $1}'`
		START=$(($END-$PERIOD))
		awk -v start=$START '{if($1 > start) {print $1,$2}}' $HERE/$AZ/$TYPE.$INSTTYPE.temp3 > $HERE/$AZ/$TYPE.$INSTTYPE.temp3a
		$BIN/bmbp_ts -f $HERE/$AZ/$TYPE.$INSTTYPE.temp3a -q 0.975 -c 0.01 -T -i 350 | grep pred > $HERE/$AZ/$TYPE.$INSTTYPE.pred
		awk '{print $2,$4,($6+0.0001)}' $HERE/$AZ/$TYPE.$INSTTYPE.pred > $HERE/$AZ/$TYPE.$INSTTYPE.temp4
		$BIN/pred-duration -T 0 -f $HERE/$AZ/$TYPE.$INSTTYPE.temp4 -e | sort -n -k 2 > $HERE/$AZ/$TYPE.$INSTTYPE.temp5
		SIZE=`wc -l $HERE/$AZ/$TYPE.$INSTTYPE.temp5 | awk '{print $1}'`
		NDX=`$BIN/bmbp_index -s $SIZE -q 0.025 -c 0.99`
		NDX=$(($NDX+1))
		echo $NDX > $HERE/$AZ/$TYPE.$INSTTYPE.ndx
		$HERE/back-test.sh $HERE/$AZ/$TYPE.$INSTTYPE.temp4 100 > $HERE/$AZ/$TYPE.$INSTTYPE.prob
	done
done

