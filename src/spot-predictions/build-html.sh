#!/bin/bash


if ( test -z "$1" ) ; then
	echo "must specify region"
	exit 1
fi

REGION=$1

HERE=/mnt/data

if ( test -e $HERE/build.$REGION.progress ) ; then
	echo `date` "build $REGION in progress"
	exit 1
fi

echo `date` > $HERE/build.$REGION.progress
echo `date` > $HERE/processing.$REGION

$HERE/split-data.sh $REGION
$HERE/generate-predictions.sh $REGION UNIX
rm -f $HERE/processing.$REGION
$HERE/make-html.sh $REGION UNIX > $HERE/$REGION.html
$HERE/make-instance-html.sh UNIX > $HERE/instance.html

cp -f $HERE/index.html /mnt/html
cp -f $HERE/table.css /mnt/html
cp -f $HERE/$REGION.html /mnt/html
cp -f $HERE/instance.html /mnt/html

rm -f $HERE/build.$REGION.progress

