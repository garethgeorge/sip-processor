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
		$HERE/pred-az.sh $AZ $TYPE $INSTTYPE &
	done
	while [ 1 ] ; do
		sleep 5
		if ( test -e "$HERE/us-east-1a-$TYPE.running" ) ; then
			continue
		fi
		if ( test -e "$HERE/us-east-1b-$TYPE.running" ) ; then
			continue
		fi
		if ( test -e "$HERE/us-east-1c-$TYPE.running" ) ; then
			continue
		fi
		if ( test -e "$HERE/us-east-1d-$TYPE.running" ) ; then
			continue
		fi
		if ( test -e "$HERE/us-east-1e-$TYPE.running" ) ; then
			continue
		fi
		break
	done
done

	

