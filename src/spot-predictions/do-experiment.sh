#!/bin/bash

if ( ! test -e $1 ) ; then
        echo "$1 does not exist"
        exit 1
fi


ECOUNT=1000
CNT=0
while ( test $CNT -lt $ECOUNT ) ; do
	./random-duration-test.sh $1
	CNT=$(($CNT+1))
done

