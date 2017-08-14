#!/bin/bash

HERE=`pwd`
BIN=/Users/rich/bin

echo `date "+%Y-%m-%d %H:%M:%S"` > $HERE/zzzdate
NOW=`$BIN/convert_time -f $HERE/zzzdate`

$BIN/phantomjs $HERE/spot_prices.js > $HERE/last-spot-price.txt

awk -v now=$NOW '{print now,$1,$2}' $HERE/last-spot-price.txt
