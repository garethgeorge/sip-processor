#!/bin/bash

PIDS=`ps auxww | grep get-aws-us | grep -v grep | awk '{print $2}'`
echo $PIDS
kill -9 $PIDS

PIDS=`ps auxww | grep generate-predictions.sh | grep -v grep | awk '{print $2}'`
kill -9 $PIDS

PIDS=`ps auxww | grep bmbp_ts | grep -v grep | awk '{print $2}'`
kill -9 $PIDS

