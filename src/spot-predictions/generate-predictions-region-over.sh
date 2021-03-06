#!/bin/bash

HERE=/mnt/data
BIN=/root/bin

REGION=$1

OVERFRAC=$2

FILE=$REGION-data.txt
INSTTYPE="Linux"

MAXSIZE=100000
PERIOD=$((2592000*3))

TARG=$HERE/results


if ( test -z "$REGION" ) ; then
echo "must input region)"
exit 1
fi

mkdir -p $TARG

NOW=`/bin/date +%s`
# 3 month earlier
EARLIEST=$(($NOW-$PERIOD))

awk '{print $3}' $FILE | sort | uniq > $HERE/instance-types.txt

for TYPE in `awk '{print $3}' $FILE | sort | uniq` ; do
	FILES=""
	for AZ in `awk '{print $2}' $FILE | sort | uniq` ; do
		grep $INSTTYPE $HERE/$AZ/$TYPE.data | grep -v SUSE | sort -k 6 | awk '{print $6,$5}' | sed 's/T/ /' | sed 's/Z//' > $HERE/$AZ/$TYPE.$INSTTYPE.temp1
		STEST=`wc -l $HERE/$AZ/$TYPE.$INSTTYPE.temp1 | awk '{print $1}'`
		if ( test $STEST -le 0 ) ; then
			continue
		fi
		$BIN/convert_time -f $HERE/$AZ/$TYPE.$INSTTYPE.temp1 | sort -n -k 1 -k 2 | awk -v early=$EARLIEST '{if($1 >= early) {print $1,$2}}' > $HERE/$AZ/$TYPE.$INSTTYPE.temp2
#		$BIN/convert_time -f $HERE/$AZ/$TYPE.$INSTTYPE.temp1 | sort -n -k 1 -k 2 > $HERE/$AZ/$TYPE.$INSTTYPE.temp2
		FILES="$HERE/$AZ/$TYPE.$INSTTYPE.temp2,$FILES"
	done
	if ( test -z "$FILES" ) ; then
		continue
	fi
#	$BIN/spot-price-aggregate -f $FILES | sort -n -k 1 -k 2 | uniq | tail -n $MAXSIZE > $TARG/$REGION-$TYPE-agg.txt
	$BIN/spot-price-aggregate -f $FILES | sort -n -k 1 -k 2 | uniq > $TARG/$REGION-$TYPE-agg.txt
	$BIN/bmbp_ts -f $TARG/$REGION-$TYPE-agg.txt -T -i 350 -q 0.975 -c 0.01 | grep "pred:" | awk -v frac=$OVERFRAC '{print $2,$4,($6+($6*frac)),$14}' > $TARG/$REGION-$TYPE-pred.txt
	awk '{print $1,$2,$3}' $TARG/$REGION-$TYPE-pred.txt > $TARG/$REGION-$TYPE-temp.txt
	$BIN/pred-duration -f $TARG/$REGION-$TYPE-temp.txt -T 0 -e | sort -n -k 2 > $TARG/$REGION-$TYPE-duration.txt
	DSIZE=`wc -l $TARG/$REGION-$TYPE-duration.txt | awk '{print $1}'`
	$BIN/bmbp_index -s $DSIZE -q 0.025 -c 0.99 > $TARG/$REGION-$TYPE-ndx.txt
	$HERE/back-test.sh $TARG/$REGION-$TYPE-pred.txt 100 > $TARG/$REGION-$TYPE-prob.txt
done

	

