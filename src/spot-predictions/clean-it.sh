#!/bin/bash

FILE=$1
AZ=$2
TYPE=$3

if ( test -z "$FILE" ) ; then
	echo "cleant_it FILE AZ TYPE"
	exit 1
fi
if ( test -z "$AZ" ) ; then
	echo "cleant_it FILE AZ TYPE"
	exit 1
fi
if ( test -z "$TYPE" ) ; then
	echo "cleant_it FILE AZ TYPE"
	exit 1
fi



grep $AZ $FILE | grep $TYPE | awk '{print $6,$5}' $1 | sed 's/T/ /' | sed 's/Z/ /' > zzz111

convert_time -f zzz111 | sort -n -k 1
