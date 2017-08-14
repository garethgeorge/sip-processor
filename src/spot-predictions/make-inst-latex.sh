#!/bin/bash

HERE=/mnt/data
BIN=/root/bin
#HERE=`pwd`
#BIN=/Users/rich/bin

EVEN=""

TARG=$HERE/results

echo "\\begin{table*}[ht]"
echo "\\centering"
echo "\\begin{tabular}{l|l|l|l}"
echo "\\hline"
echo "Inst. Type" '&' "us-east-1" '&' "us-west-1" '&' "us-west-2" "\\\\"
echo "\\hline\\hline"
for INSTTYPE in `cat $HERE/instances.txt` ; do
	echo -n $INSTTYPE
	for REGION in `cat $HERE/regions2.txt` ; do
		if ( test -e $HERE/processing.$REGION ) ; then
                        if ( test -e $TARG/$REGION-$INSTTYPE.vdata ) ; then
                                VDATA=`cat $TARG/$REGION-$INSTTYPE.vdata | awk '{printf "%10.0f %2.4f %0.2f", $1,$2,$3}'`
                        else
                                VDATA=`tail -n 1 $TARG/$REGION-$INSTTYPE-pred.txt | awk '{printf "%10.0f %2.4f %0.2f", $1,$3,$4}'`
                        fi
                        if ( test -e $TARG/$REGION-$INSTTYPE.ddata ) ; then
                                DDATA=`cat $TARG/$REGION-$INSTTYPE.ddata`
                        else
                                NDX=`cat $TARG/$REGION-$INSTTYPE-ndx.txt`
                                DDATA=`head -n $NDX $TARG/$REGION-$INSTTYPE-duration.txt | tail -n 1 | awk '{printf "%0.2f",$2/3600}'`
                        fi
		else
			VDATA=`tail -n 1 $TARG/$REGION-$INSTTYPE-pred.txt | awk '{printf "%10.0f %2.4f %0.2f", $1,$3,$4}'`
			NDX=`cat $TARG/$REGION-$INSTTYPE-ndx.txt`
			DDATA=`head -n $NDX $TARG/$REGION-$INSTTYPE-duration.txt | tail -n 1 | awk '{printf "%0.2f",$2/3600}'`
		fi

		TTIME=`echo $VDATA | awk '{print $1}'`
		PTIME=`$BIN/ptime $TTIME`
		BID=`echo $VDATA | awk '{print $2}'`
		BIDP=`echo $VDATA | awk '{print $3}'`
		DUR=`echo $DDATA | awk '{print $1}'`
		DURP=`cat $TARG/$REGION-$INSTTYPE-prob.txt`
		if ( test "$BID" != "0.0000" ) ; then
			echo -n " " '& \$'"$BID," "\textbf{$DUR (hrs)}," "\textit{$DURP}"
		else
			echo -n " " '&'
		fi
	done
	echo "\\\\"
	echo "\\hline"
done

echo "\\end{tabular}"
echo "\\caption{DAFTS predictions for AWS instance types in \textit{us-east-1}, \textit{us-west-1}, and \textit{us-west-2} gathered on XXX.  Missing entries indicate data not available via the AWS price history API.\label{tab:pred}}"
echo "\\end{table*}"

