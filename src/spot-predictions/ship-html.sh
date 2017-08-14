#!/bin/bash

if ( test -z "$1" ) ; then
	echo "must specify region"
	exit 1
fi

HERE=`pwd`

$HERE/split-data.sh $REGION
$HERE/generate-predictions.sh $REGION UNIX
$HERE/make-html.sh $REGION UNIX > $REGION.html
HOST=128.111.84.197

scp -i /home/rich/euca-eci/rich.key $HERE/index.html root@$HOST:/mnt/html
scp -i /home/rich/euca-eci/rich.key $HERE/table.css root@$HOST:/mnt/html
scp -i /home/rich/euca-eci/rich.key $HERE/$REGION.html root@$HOST:/mnt/html

