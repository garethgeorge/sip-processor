#!/bin/bash

HERE=`pwd`
BIN=/home/rich/bin

REGION=$1

FILE=$REGION-vpc-data.txt
INSTTYPE="LinuxVPC"

MAXSIZE=100000
PERIOD=$((2592000*3))

TARG=$HERE/results

GMTOFF=`date --rfc-3339="seconds" | awk -F '-' '{print $4}' | awk -F':' '{printf "%2d", $1}' | sed 's/ //'`


if ( test -z "$REGION" ) ; then
echo "must input region)"
exit 1
fi

mkdir -p $TARG

NOW=`/bin/date +%s`
NOW=$(($NOW-($GMTOFF*3600)))

# 3 month earlier
EARLIEST=$(($NOW-$PERIOD))

#awk '{print $3}' $FILE | sort | uniq > $HERE/instance-types.txt

#for TYPE in `awk '{print $3}' $FILE | sort | uniq` ; do
for TYPE in c3.2xlarge c3.4xlarge c3.8xlarge r3.2xlarge r3.4xlarge r3.8xlarge g2.2xlarge g2.8xlarge m3.2xlarge ; do
#	for AZ in `awk '{print $2}' $FILE | sort | uniq` ; do
	for AZ in us-east-1a us-east-1b us-east-1c us-east-1d us-east-1e ; do
		grep $INSTTYPE $HERE/$AZ/$TYPE.data | grep -v SUSE | sort -k 6 | awk '{print $6,$5}' | sed 's/T/ /' | sed 's/Z//' > $HERE/$AZ/$TYPE.$INSTTYPE.temp1
                STEST=`wc -l $HERE/$AZ/$TYPE.$INSTTYPE.temp1 | awk '{print $1}'`
                if ( test $STEST -le 0 ) ; then
                        continue
                fi
		$BIN/convert_time -f $HERE/$AZ/$TYPE.$INSTTYPE.temp1 | sort -n -k 1 -k 2  > $HERE/$AZ/$TYPE.$INSTTYPE.temp0
		awk -v gmt=$GMTOFF '{print $1-(gmt*3600),$2}' $HERE/$AZ/$TYPE.$INSTTYPE.temp0 | awk -v early=$EARLIEST '{if($1 >= early) {print $1,$2}}' > $HERE/$AZ/$TYPE.$INSTTYPE.temp2
		$BIN/bmbp_ts -f $HERE/$AZ/$TYPE.$INSTTYPE.temp2 -T -q 0.99 -c 0.01 -i 350 | grep "pred:" | awk '{print $2,$4,$6+0.0001,$14}'> $HERE/$AZ/$TYPE.$INSTTYPE.pred
		awk '{print $1,$2,$3}' $HERE/$AZ/$TYPE.$INSTTYPE.pred > $HERE/$AZ/$TYPE.$INSTTYPE.temp3
		$BIN/pred-distribution -f $HERE/$AZ/$TYPE.$INSTTYPE.temp3 -q 0.01 -c 0.99 -F 4.0 -I 0.05 | awk '{print $1/3600,$2}' > $TARG/$AZ-$TYPE.pgraph
	done
done

	

