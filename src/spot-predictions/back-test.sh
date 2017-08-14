#!/bin/bash

# checks prediction against specific AZ

FILE=$1
SAMPLESIZE=$2

HERE=/mnt/data
BIN=/root/bin

PERIOD=$((2592000*1))
#PERIOD=$((604800*1))

MINSIZE=375

RANDOM=$$

if ( test -z "$1" ) ; then
	echo "must specify region"
	exit 1
fi


if ( test -z "$2" ) ; then
	echo "must specify sample size"
	exit 1
fi

LATEST=`tail -n 1 $FILE | awk '{print $1}'`

EARLIEST=`head -n 1 $FILE | awk '{print $1}'`
EARLIEST=$(($EARLIEST+$PERIOD))

if ( test $EARLIEST -gt $LATEST ) ; then
	exit 1
fi
RANGE=$(($LATEST-$EARLIEST))

ATTEMPT=1

MISS=0
SAMPCNT=0
while ( test $SAMPCNT -lt $SAMPLESIZE ) ; do

if ( test $ATTEMPT -ge 100 ) ; then
#	echo BAD 100 attempts
	exit 1
fi

CUTOFF=`echo $RANDOM $RANGE | awk '{printf "%10.0f",($1/32768)*$2}'`

CUTTS=$(($EARLIEST+$CUTOFF))

awk -v cut=$CUTTS -v early=$EARLIEST '{if(($1 < cut) && ($1 > early)) {print $1,$2,$3}}' $FILE > $FILE.in.agg
awk -v cut=$CUTTS '{if($1 > cut) {print $1,$2}}' $FILE > $FILE.out.agg

MIN=`tail -n 1 $FILE.in.agg | awk '{print $3}'`

ENDTIME=`awk -v min=$MIN '{if($2 >= min) {print $1}}' $FILE.out.agg | sort -n -k 1 | head -n 1` 

# skip if we don't get an end time for prediction
if ( test -z "$ENDTIME" ) ; then
	ENDTIME=`tail -n 1 $FILE.out.agg | awk '{print $1}'`
#	echo skipping $ENDTIME
#	ATTEMPT=$(($ATTEMPT+1))
#	continue 
fi

ELAPSED=$(($ENDTIME-$CUTTS))

$BIN/pred-duration -f $FILE.in.agg -T 0 -e | sort -n -k 2 > $FILE.dur.agg
DTEST=`wc -l $FILE.dur.agg | awk '{print $1}'`
if ( test $DTEST -le 0 ) ; then
#	echo BAD skipping durpred at $CUTTS
	ATTEMPT=$(($ATTEMPT+1))
	continue
fi
SIZE=`wc -l $FILE.dur.agg | awk '{print $1}'`
if ( test $SIZE -lt $MINSIZE) ; then
#	echo BAD skipping becaiuse $SIZE less than $MINSIZE
	ATTEMPT=$(($ATTEMPT+1))
	continue
fi

NDX=`$BIN/bmbp_index -s $SIZE -q 0.025 -c 0.99`
NDX=$(($NDX+1))
DURPRED=`head -n $NDX $FILE.dur.agg | tail -n 1 | awk '{print $2}'`

if ( test $ELAPSED -lt $DURPRED ) ; then
	MISS=$(($MISS+1))
fi

SAMPCNT=$(($SAMPCNT+1))
ATTEMPT=1
done

echo $MISS $SAMPCNT | awk '{printf "%0.2f",1.0 - ($1/$2)}'


