#!/bin/bash

BIN=/root/bin
HERE=/mnt/data

REGION=$1

if ( test -z "$REGION" ) ; then
	echo "must specify region"
	exit 1
fi

FILE="$HERE/$REGION-data.txt"

awk '{print $2}' $FILE | sort | uniq > $HERE/$REGION.az.txt


for AZ in `cat $HERE/$REGION.az.txt` ; do
	mkdir -p $AZ
	rm -rf $AZ/*.data
	for TYPE in `cat $HERE/instance-types.txt` ; do
		echo $AZ $TYPE
		grep $AZ $FILE | grep $TYPE | grep Linux | grep -v SUSE > $HERE/$AZ/$TYPE.data
	done
done

