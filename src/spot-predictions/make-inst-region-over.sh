#!/bin/bash

HERE=/mnt/data
BIN=/root/bin
#HERE=`pwd`
#BIN=/Users/rich/bin

MYREGION=$1
OVERFRAC=$2

EVEN=""

TARG=$HERE/results

echo '<html xmlns="http://www.w3.org/1999/xhtml" lang="en" xml:lang="en">'
echo "<head>"
	echo '<meta http-equiv="content-type" content="text/html; charset=utf-8" />'
	echo '<meta name="description" content="AWS Spot Bids by Region" />'
	echo '<link href='http://fonts.googleapis.com/css?family=Dosis' rel='stylesheet' type='text/css' />'
	echo '<link rel="stylesheet" type="text/css" href="./table.css" title="Table" media="all" />'
	echo '<title>AWS Spot Bids by AZ</title>'
echo "</head>"
echo '<body class="light blue smaller freestyle01">'

NOW=`date`

PCT=`echo $OVERFRAC | awk '{printf "%d",$1*100}'`

echo "<h2>DAFTS Minimum +" $PCT"% Spot Price Predictions by Instance Type</h2>"
echo "<h3>$NOW</h3>"

for INSTTYPE in `cat $HERE/instances.txt` ; do
	echo "<h2>$INSTTYPE</h2>"
	echo "<table cellspacing='0'>"
	echo "<thead>"
	echo "<tr>"
		echo "<th>Region</th>"
		echo "<th>Time</th>"
		echo "<th>Bid $</th>"
		echo "<th>Duration</th>"
	echo "</tr>"
	echo "</thead>"
	echo "<tbody>"
	for REGION in `cat $HERE/regions2.txt` ; do
                if ( test "$MYREGION" != "$REGION" ) ; then
                        if ( test -e $TARG/$REGION-$INSTTYPE.vdata ) ; then
                                VDATA=`cat $TARG/$REGION-$INSTTYPE.vdata`
                        else
				VDATA=`tail -n 1 $TARG/$REGION-$INSTTYPE-pred.txt | awk '{printf "%10.0f %7.4f %0.2f", $1,$3,$4}'`
                        fi
                        if ( test -e $TARG/$REGION-$INSTTYPE.ddata ) ; then
                                DDATA=`cat $TARG/$REGION-$INSTTYPE.ddata`
                        else
#				DDATA=`tail -n 1 $TARG/$REGION-$INSTTYPE-durpred.txt | awk '{printf "%10.0f %7.2f %0.2f", $1,$3/3600,$4}'`
				NDX=`cat $TARG/$REGION-$INSTTYPE-ndx.txt`
				DDATA=`head -n $NDX $TARG/$REGION-$INSTTYPE-duration.txt | tail -n 1 | awk '{printf "%0.2f",$2/3600}'`
                        fi
                else
			VDATA=`tail -n 1 $TARG/$REGION-$INSTTYPE-pred.txt | awk '{printf "%10.0f %7.4f %0.2f", $1,$3,$4}'`
#			DDATA=`tail -n 1 $TARG/$REGION-$INSTTYPE-durpred.txt | awk '{printf "%10.0f %7.2f %0.2f", $1,$3/3600,$4}'`
			NDX=`cat $TARG/$REGION-$INSTTYPE-ndx.txt`
			DDATA=`head -n $NDX $TARG/$REGION-$INSTTYPE-duration.txt | tail -n 1 | awk '{printf "%0.2f",$2/3600}'`
                fi

		TTIME=`echo $VDATA | awk '{print $1}'`
		PTIME=`$BIN/ptime $TTIME`
		BID=`echo $VDATA | awk '{print $2}'`
		BIDP=`echo $VDATA | awk '{print $3}'`
		DUR=`echo $DDATA | awk '{print $1}'`
#		DURP=`echo $DDATA | awk '{print $3}'`
		DURP=`cat $TARG/$REGION-$INSTTYPE-prob.txt`
                if ( test "$MYREGION" = "$REGION" ) ; then
			echo $VDATA > $TARG/$REGION-$INSTTYPE.vdata
			echo $DDATA > $TARG/$REGION-$INSTTYPE.ddata
		fi
		if ( ! test -z "$EVEN" ) ; then
			echo '<tr class="even">'
			EVEN=""
		else
			echo "<tr>"
			EVEN="1"
		fi
			echo "<td>$REGION</td>"
			echo "<td>$PTIME</td>"
			if ( test -z "$BID" ) ; then
				BID="none"
			fi
			if ( test -z "$BIDP" ) ; then
				BIDP="none"
			fi
			echo "<td>\$$BID ($BIDP)</td>"
			if ( test -z "$DUR" ) ; then
				DUR="none"
			fi
			if ( test -z "$DURP" ) ; then
				DURP="none"
			fi
			echo "<td>$DUR hrs ($DURP)</td>"
		echo "</tr>"
	done
	echo "</tbody>"
	echo "</table>"
done

echo "</body>"
echo "</html>"

