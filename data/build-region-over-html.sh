#!/bin/bash

REGION=$1
OVERFRAC=$2


if ( test -z "$1" ) ; then
	echo "must specify region"
	exit 1
fi

HERE=/mnt/data

if ( test -e $HERE/$REGION.buildregion.progress ) ; then
	echo `date` "build $REGION in progress"
	exit 1
fi

echo `date` > $HERE/$REGION.buildregion.progress
echo `date` > $HERE/processing.$REGION

$HERE/split-data.sh $REGION
$HERE/generate-predictions-region-over.sh $REGION $OVERFRAC

rm -f $HERE/processing.$REGION

$HERE/make-region-over-html.sh $REGION $OVERFRAC  > $HERE/$REGION.new.html
$HERE/make-inst-region-over.sh $REGION $OVERFRAC  > $HERE/$REGION.by-inst.html
#$HERE/make-inst-latex.sh > $HERE/pred.tab

#cp -f $HERE/index.html /mnt/html
cp -f $HERE/table.css /mnt/html
cp -f $HERE/$REGION.new.html /mnt/html
cp -f $HERE/$REGION.new.html /mnt/html/by-region.html
cp -f $HERE/$REGION.by-inst.html /mnt/html/by-inst.html

rm -f $HERE/$REGION.buildregion.progress

