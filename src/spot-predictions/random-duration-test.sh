#!/bin/bash

FILE=$1

if ( ! test -e $FILE ) ; then
	echo "$FILE does not exist"
	exit 1
fi

SIZE=`wc -l $FILE | awk '{print $1}'`

MOMENT=$(($RANDOM % $SIZE + 100))

while ( test $MOMENT -ge $SIZE ) ; do
	MOMENT=$(($RANDOM % $SIZE + 100))
done

head -n $MOMENT $FILE | awk '{print $1,$2}' > temp1.$FILE
bmbp_ts -f temp1.$FILE -T -q 0.975 -c 0.025 | grep "pred" | awk '{print $2,$4,$6,$14}' > temp2.$FILE
awk '{print $1,$2,$3}' temp2.$FILE > temp3.$FILE
pred-duration -f temp3.$FILE | sort -n -k 1 > temp4.$FILE
bmbp_ts -f temp4.$FILE -T -q 0.025 -c 0.975 -L | grep "pred" > temp5.$FILE
DSIZE=`wc -l temp4.$FILE | awk '{print $1}'`
PREDICTION=`tail -n 1 temp2.$FILE | awk '{print $3}'`
DURATION=`tail -n 1 temp5.$FILE | awk '{printf "%7.0f", $6}'`
PFRAC=`tail -n 1 temp2.$FILE | awk '{print $4}'`
DFRAC=`tail -n 1 temp5.$FILE | awk '{print $14}'`
#QNDX=`echo $MOMENT $PFRAC | awk '{printf "%7.0f",$1*$2}'`
QNDX=`echo $MOMENT "0.975" | awk '{printf "%7.0f",$1*$2}'`
DNDX=`echo $DSIZE "0.975" | awk '{printf "%7.0f",$1*(1.0-$2)}'`
#DNDX=`echo $DSIZE $DFRAC | awk '{printf "%7.0f",$1*(1.0-$2)}'`
QVALUE=`awk '{print $2}' temp1.$FILE | sort -n | head -n $QNDX | tail -n 1`
DVALUE=`awk '{print $2}' temp4.$FILE | sort -n | head -n $DNDX | tail -n 1 | awk '{printf "%10.0f",$1}'`
#echo $QNDX $QVALUE $DNDX $DVALUE
#exit 1

#echo $MOMENT $PREDICTION $DURATION

REST=$(($SIZE - $MOMENT))

tail -n $REST $FILE > temp6.$FILE
FIRST=`head -n 1 temp6.$FILE | awk '{print $1}'`
LAST=`tail -n 1 temp2.$FILE | awk '{print $1}'`
INCR=$(($RANDOM % ($FIRST-$LAST)))
#NOW=$(($LAST+$INCR))
NOW=$LAST

STATUS=SUCCESS
for CURR in `awk '{print $1}' temp6.$FILE` ; do
	VALUE=`grep $CURR temp6.$FILE | awk '{print $2}'`
	TESTPFAIL=`echo $VALUE $PREDICTION | awk '{if ( $1 > $2 ) print 1; else print 0;}'`
	DIFF=$(($CURR - $NOW))
	if ( test $TESTPFAIL -eq 1 ) ; then
		if ( test $DIFF -lt $DURATION ) ; then
			STATUS=FAIL
		else
			STATUS=SUCCESS 
		fi
		break;
	fi
done
echo QBETS $STATUS $PFRAC $DFRAC "($NOW $VALUE $PREDICTION $DIFF $DURATION)"


STATUS=SUCCESS
for CURR in `awk '{print $1}' temp6.$FILE` ; do
	VALUE=`grep $CURR temp6.$FILE | awk '{print $2}'`
	TESTPFAIL=`echo $VALUE $QVALUE | awk '{if ( $1 > $2 ) print 1; else print 0;}'`
	DIFF=$(($CURR - $NOW))
	if ( test $TESTPFAIL -eq 1 ) ; then
		if ( test $DIFF -lt $DVALUE ) ; then
			STATUS=FAIL
		else
			STATUS=SUCCESS 
		fi
		break;
	fi
done

#echo QUANTILE $STATUS $PFRAC $DFRAC "($NOW $VALUE $QVALUE $DIFF $DVALUE)"
echo QUANTILE $STATUS "0.975 0.975 ($NOW $VALUE $QVALUE $DIFF $DVALUE)"
