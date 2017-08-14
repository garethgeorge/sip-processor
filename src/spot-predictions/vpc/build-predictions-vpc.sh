#!/bin/bash

REGION=$1


if ( test -z "$1" ) ; then
	echo "must specify region"
	exit 1
fi

if ( test "$REGION" != "us-east-1" ) ; then
	exit 0
fi

HERE=`pwd`
HTML=/local/html/vpc

if ( test -e $HERE/$REGION.buildregion.progress ) ; then
	echo `date` "build $REGION in progress"
	exit 1
fi

echo `date` > $HERE/$REGION.buildregion.progress

FILE=$REGION-vpc-data.txt
$HERE/split-vpc-data.sh $REGION
#$HERE/generate-predictions-vpc-region.sh $REGION
$HERE/generate-predictions-vpc-az.sh $REGION

#$HERE/make-region-vpc.html $REGION
#for AZ in `awk '{print $2}' $FILE | sort | uniq` ; do
for AZ in us-east-1a us-east-1b us-east-1c us-east-1d us-east-1e; do
	$HERE/make-az-vpc.html $AZ
done
#$HERE/make-inst-region.sh > $HERE/$REGION.by-inst.html

#cp -f $HERE/index.html $HTML
#cp -f $HERE/table.css $HTML
#cp -f $HERE/$REGION.new.html $HTML/by-region.html
#cp -f $HERE/$REGION.by-inst.html $HTML/by-inst.html

rm -f $HERE/$REGION.buildregion.progress

