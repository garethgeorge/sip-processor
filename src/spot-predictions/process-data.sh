#!/bin/bash

HERE=/mnt/data

for REGION in `cat $HERE/regions.txt` ; do
	$HERE/build-html.sh $REGION
done

