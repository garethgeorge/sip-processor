#!/bin/bash

grep OUTLIER $1 > zzzout
RUNS=`grep RUN $1 | awk '{print $1}' | sort | uniq`
for RUN in $RUNS ; do
	START=`grep "$RUN" $1 | awk '{print $2}' | sort -n | head -n 1`
	END=`grep "$RUN" $1 | awk '{print $2}' | sort -n | tail -n 1`
# first OUTLIER in each run is right censored
# for empht, 0 in first colum implies censored
	CENSORED=0
	LAST=$START
	FOUND=0
	for OUT in `grep OUTLIER zzzout | awk '{print $2}' | sort -n` ; do
		if ( test $OUT -ge $START ) ; then
			if ( test $OUT -le $END ) ; then
				INTV=`echo $OUT $LAST | awk '{print $1-$2}'`
				echo $CENSORED $INTV
				CENSORED=1
				LAST=$OUT
				FOUND=1
			fi
		fi
	done
#	if ( test $FOUND -eq 1 ) ; then
#		INTV=`echo $END $LAST | awk '{print $1-$2}'`
#		CENSORED=0
#		echo $CENSORED $INTV
#	fi
done
echo -1

