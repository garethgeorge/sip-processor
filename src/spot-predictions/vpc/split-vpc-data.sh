#!/bin/bash

BIN=/home/rich/bin
HERE=`pwd`

REGION=$1

if ( test -z "$REGION" ) ; then
	echo "must specify region"
	exit 1
fi

FILE="$HERE/$REGION-vpc-data.txt"

awk '{print $3}' $FILE | sort | uniq > $HERE/instance-types.txt


for AZ in `cat $HERE/$REGION.az.txt` ; do
	mkdir -p $AZ
	rm -rf $AZ/*.data
	for TYPE in `cat $HERE/instance-types.txt` ; do
		echo $AZ $TYPE
		grep $AZ $FILE | grep $TYPE | grep Linux | grep -v SUSE | sed 's/VPC)/VPC)/ ' | sed 's$Linux/UNIX (Amazon VPC)$LinuxVPC$' > $HERE/$AZ/$TYPE.data
	done
done

